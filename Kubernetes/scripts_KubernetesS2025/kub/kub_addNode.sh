#!/bin/bash
# ---------------------------------------------------------------------
# Script: kub_addNode.sh
# Purpose: Automatically add a new Kubernetes worker node to the cluster.
# ---------------------------------------------------------------------

KEYFILE="./labsuser.pem"

# --- Pre-flight checks ---
if [ ! -f "$KEYFILE" ]; then
    echo "❌ Missing SSH key: $KEYFILE"
    exit 1
fi

# Check key file permissions
#echo "🔑 Verifying key file permissions..."
PERMS=$(stat -c "%a" "$KEYFILE")
if [ "$PERMS" != "400" ]; then
    echo "🚫 Error: $KEYFILE has incorrect permissions ($PERMS)."
    echo "Please run this and try again: chmod 400 $KEYFILE"
    echo "Exitting..."
    exit 1
fi

if [ -z "$1" ]; then
    echo "Usage: ./kub_addNode.sh <NEW_WORKER_IP>"
    exit 1
fi

NEWWORKER=$1
HOST=$(hostname)

# Must run on the master
if [[ "$HOST" != *"k8smaster"* ]]; then
    echo "🚫 ERROR: This script must be executed from the control-plane node."
    exit 1
fi

#if [ ! -f "/etc/kubernetes/admin.conf" ]; then
#    echo "🚫 ERROR: This script must be executed from the control-plane node."
#    echo "   (Master configuration /etc/kubernetes/admin.conf not found)"
#    exit 1
#fi

MASTER_IP=$(hostname -I | awk '{print $1}')       # 💡 Get master IP
MASTER_HOST=$(hostname)                           # 💡 Get master hostname

# ---------------------------------------------------------------------
# Determine correct worker index (reuse if IP already present)
# ---------------------------------------------------------------------
EXISTING_ENTRY=$(grep -E "^${NEWWORKER}[[:space:]]" /etc/hosts)

# Get a sorted list of all currently used slave indices
ALL_INDICES=$(grep -oP 'k8sslave\K[0-9]+' /etc/hosts | sort -n)

if [ -n "$EXISTING_ENTRY" ]; then
    # IP already exists — extract existing slave index
    EXISTING_INDEX=$(echo "$EXISTING_ENTRY" | grep -oP 'k8sslave\K[0-9]+')
    if [ -n "$EXISTING_INDEX" ]; then
        # This IP is already a known slave. Use its existing index.
        NUMWORKERS=$EXISTING_INDEX
        echo "♻️  Worker IP ${NEWWORKER} already present as k8sslave${NUMWORKERS}.psdi.org — reusing index."
        # Clean up any duplicate lines for this IP before reusing
        sudo sed -i "/^${NEWWORKER}[[:space:]]/d" /etc/hosts
    else
        # IP exists but has no valid slave name. Clean it and assign the first available gap.
        echo "⚠️  Existing entry for ${NEWWORKER} has no slave tag — cleaning up and assigning first available gap."
        sudo sed -i "/^${NEWWORKER}[[:space:]]/d" /etc/hosts

        NUMWORKERS=1
        while echo "$ALL_INDICES" | grep -q -w "$NUMWORKERS"; do
            NUMWORKERS=$((NUMWORKERS + 1))
        done
    fi
else
    # New IP — find the first available index (gap-filling logic)
    NUMWORKERS=1
    while echo "$ALL_INDICES" | grep -q -w "$NUMWORKERS"; do
        # If grep finds the number (exit code 0), increment and check the next number
        NUMWORKERS=$((NUMWORKERS + 1))
    done
    # The loop stops when grep *fails* (exit code 1), meaning $NUMWORKERS is the first gap.
fi

echo ">>>"
echo "👷‍♂️ Detected next available worker index: $NUMWORKERS"
echo "👷‍♂️ Adding new worker as k8sslave${NUMWORKERS}.psdi.org ($NEWWORKER)"

# --- Copy installer scripts and key to worker -------------------------
echo ">>>"
echo "📦 Copying installer scripts and .pem to worker..."
ssh -o StrictHostKeyChecking=no -i "$KEYFILE" ubuntu@$NEWWORKER "mkdir -p ~/kub"
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "chmod u+w /home/ubuntu/kub/$KEYFILE 2>/dev/null || true"
scp -i "$KEYFILE" -q *.sh *.pem ubuntu@$NEWWORKER:/home/ubuntu/kub/
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "chmod 400 /home/ubuntu/kub/$KEYFILE"
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "chmod +x /home/ubuntu/kub/*.sh"

# --- 💡 Ensure master entry is present in new worker’s /etc/hosts before install ---
echo ">>>"
echo "➕ Adding master entry to worker /etc/hosts..."
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "echo '${MASTER_IP} ${MASTER_HOST}' | sudo tee -a /etc/hosts"

# --- Install Kubernetes prerequisites on worker ------------------------
echo ">>>"
echo "🧱 Installing Kubernetes prerequisites on the worker node..."
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "cd ~/kub && ./kub_install.sh $NUMWORKERS"

# ---------------------------------------------------------------------
# Ensure correct hostname mapping on new worker
# ---------------------------------------------------------------------
MASTER_IP=$(grep "k8smaster" /etc/hosts | awk '{print $1}')
MASTER_HOST=$(grep "k8smaster" /etc/hosts | awk '{print $2}')

echo ">>>"
echo "➕ Ensuring master entry exists in worker /etc/hosts..."
# 🧹 Remove any line with same IP before adding
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "sudo sed -i \"/^${MASTER_IP}[[:space:]]/d\" /etc/hosts"
# 🧩 Add or re-add master entry cleanly
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "grep -q '${MASTER_HOST}' /etc/hosts || echo '${MASTER_IP} ${MASTER_HOST}' | sudo tee -a /etc/hosts > /dev/null"

# ---------------------------------------------------------------------
# Propagate known slaves from master’s /etc/hosts
# ---------------------------------------------------------------------
echo ">>>"
echo "🔁 Propagating known cluster entries to new worker..."
grep 'k8s' /etc/hosts | while read -r line; do
    IP=$(echo "$line" | awk '{print $1}')
    # 🧹 Remove any previous mapping with same IP
    ssh -i "$KEYFILE" ubuntu@$NEWWORKER "sudo sed -i \"/^${IP}[[:space:]]/d\" /etc/hosts"
    # 🧩 Add the fresh line
    ssh -i "$KEYFILE" ubuntu@$NEWWORKER "echo '${line}' | sudo tee -a /etc/hosts > /dev/null"
done

# --- Join cluster -----------------------------------------------------
echo ">>>"
echo "🤝 Joining the worker node to the Kubernetes cluster. 🤞 Cross your fingers!"
JOIN_CMD=$(kubeadm token create --print-join-command)
ssh -i "$KEYFILE" ubuntu@$NEWWORKER "sudo $JOIN_CMD"

# --- Verify and label node --------------------------------------------
echo ">>>"
echo "🔍 Verifying cluster node status..."
if kubectl get nodes -o wide | grep -q "k8sslave${NUMWORKERS}.psdi.org"; then
    echo "✅ Node k8sslave${NUMWORKERS}.psdi.org successfully joined!"
    echo ">>>"
    echo "➕ Adding worker entry to master and other nodes..."

    # 🧹 Clean up any existing line with same IP locally before adding
    sudo sed -i "/^${NEWWORKER}[[:space:]]/d" /etc/hosts
    echo "${NEWWORKER} k8sslave${NUMWORKERS}.psdi.org" | sudo tee -a /etc/hosts > /dev/null

    for NODE_IP in $(grep -oP '^172\.\d+\.\d+\.\d+' /etc/hosts | sort -u); do
        if [[ "$NODE_IP" == "$(hostname -I | awk '{print $1}')" ]]; then
            continue
        fi
        echo "   ↪ Updating $NODE_IP ..."
        # 🧹 Clean up old mapping for same IP on remote node
        ssh -i "$KEYFILE" ubuntu@$NODE_IP "sudo sed -i \"/^${NEWWORKER}[[:space:]]/d\" /etc/hosts"
        # 🧩 Add or re-add correct entry
        ssh -i "$KEYFILE" ubuntu@$NODE_IP "echo '${NEWWORKER} k8sslave${NUMWORKERS}.psdi.org' | sudo tee -a /etc/hosts > /dev/null"
    done
else
    echo "⚠️ Node not yet visible in cluster (might take a few seconds to register)"
fi

# ---------------------------------------------------------------------
# 🧩 Final consistency sync: propagate full master /etc/hosts to new worker
# ---------------------------------------------------------------------
echo ">>>"
echo "🔁 Performing final consistency sync for /etc/hosts on ${NEWWORKER}..."
grep 'k8s' /etc/hosts | while read -r line; do
    IP=$(echo "$line" | awk '{print $1}')
    ssh -i "$KEYFILE" ubuntu@$NEWWORKER "sudo sed -i \"/^${IP}[[:space:]]/d\" /etc/hosts"
    ssh -i "$KEYFILE" ubuntu@$NEWWORKER "echo '${line}' | sudo tee -a /etc/hosts > /dev/null"
done

# ---------------------------------------------------------------------
# 🧩 Reciprocal sync: propagate the new worker entry to all other nodes
# ---------------------------------------------------------------------

for NODE_IP in $(grep -oP '^172\.\d+\.\d+\.\d+' /etc/hosts | sort -u); do
    if [[ "$NODE_IP" == "$(hostname -I | awk '{print $1}')" ]]; then
        continue
    fi
    ssh -i "$KEYFILE" ubuntu@$NODE_IP \
        "sudo sed -i \"/^${NEWWORKER}[[:space:]]/d\" /etc/hosts && echo '${NEWWORKER} k8sslave${NUMWORKERS}.psdi.org' | sudo tee -a /etc/hosts > /dev/null"
done

# ---------------------------------------------------------------------
# 🧹 Deduplicate /etc/hosts across all nodes
# ---------------------------------------------------------------------
echo ">>>"
echo "🧹 Deduplicating /etc/hosts across cluster..."
for NODE_IP in $(grep -oP '^172\.\d+\.\d+\.\d+' /etc/hosts | sort -u); do
    ssh -i "$KEYFILE" ubuntu@$NODE_IP \
        "sudo awk '!seen[\$0]++' /etc/hosts > /tmp/hosts && sudo mv /tmp/hosts /etc/hosts"
done

echo ">>>"
echo "🏷️ Labeling node k8sslave${NUMWORKERS}.psdi.org as worker..."
kubectl label node "k8sslave${NUMWORKERS}.psdi.org" node-role.kubernetes.io/worker=worker --overwrite || true

echo ">>>"
echo "✅ Node k8sslave${NUMWORKERS}.psdi.org added and labeled successfully!"