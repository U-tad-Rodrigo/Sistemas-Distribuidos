#!/bin/bash
# ---------------------------------------------------------------------
# Script: kub_deploy.sh
# Purpose: Fully idempotent Kubernetes cluster orchestrator
# Supports: Add, remove, and inspect worker nodes safely
# ---------------------------------------------------------------------

echo "==============================================="
echo "🚀 Kubernetes Cluster Deployment Orchestrator"
echo "==============================================="
echo

set -euo pipefail
IFS=$'\n\t'

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

KUBECONFIG_ADMIN="/etc/kubernetes/admin.conf"
KUBECONFIG_NODE="/etc/kubernetes/kubelet.conf"

if [ ! -f "$KUBECONFIG_ADMIN" ] && [ -f "$KUBECONFIG_NODE" ]; then
    echo "⚠️  This should be run on the control-plane node (master)"
    exit 1
fi

HOSTNAME=$(hostname)
echo "🏷️  Detected host: $HOSTNAME"
echo

# --- Function: Check master health ---
is_master_healthy() {
    kubectl get nodes &>/dev/null
}

# --- Function: Check if a worker already exists ---
worker_exists() {
    local ip="$1"
    local existing_ips
    existing_ips=$(kubectl get nodes -o wide --no-headers | awk '{print $6}')
    echo "🔎 Existing node IPs: $existing_ips"
    echo "$existing_ips" | grep -Fxq "$ip"
}

# --- Function: Remove worker node ---
remove_worker() {
    local target_ip="$1"
    echo "🧹 Initiating removal of worker node with IP: $target_ip"

    # Find hostname of node by IP
    local node_name
    node_name=$(kubectl get nodes -o wide --no-headers | grep "$target_ip" | awk '{print $1}') || true

    if [ -z "${node_name:-}" ]; then
        echo "⚠️ No node found with IP $target_ip."
        return
    fi

    # Ask for confirmation
    read -p "❓ Are you sure you want to remove node $node_name ($target_ip)? [y/N]: " CONFIRM_REMOVE
    if [[ ! "$CONFIRM_REMOVE" =~ ^[Yy]$ ]]; then
        echo "⏭️  Skipping removal of $target_ip."
        return
    fi

    echo "🚫 Draining node $node_name..."
    kubectl drain "$node_name" --ignore-daemonsets --delete-emptydir-data --force || true

    echo "🗑️  Deleting node $node_name from cluster..."
    kubectl delete node "$node_name" || true

    echo "🧼 Cleaning /etc/hosts entries for $target_ip across cluster..."
    sudo sed -i "/${target_ip}/d" /etc/hosts

    for NODE_IP in $(grep -oP '^172\.\d+\.\d+\.\d+' /etc/hosts | sort -u); do
        if [[ "$NODE_IP" == "$(hostname -I | awk '{print $1}')" ]]; then
            continue
        fi
        echo "   ↪ Removing $target_ip from $NODE_IP ..."
        ssh -i "$KEYFILE" -o StrictHostKeyChecking=no ubuntu@$NODE_IP \
            "sudo sed -i '/${target_ip}/d' /etc/hosts" 2>/dev/null || \
            echo "   ⚠️ Could not clean on $NODE_IP"
    done

    echo "💣 Attempting remote cleanup on $target_ip..."
    ssh -i "$KEYFILE" -o StrictHostKeyChecking=no ubuntu@$target_ip \
        "bash -s" < ./kub_reset.sh || echo "⚠️ Could not reach $target_ip for cleanup."

    echo "✅ Node $node_name ($target_ip) removed successfully."
}

# --- Detect cluster state ---
if is_master_healthy; then
    echo "✅ Control-plane (master node) already initialized."
else
    echo "👑 Control-plane (master) not detected."
    read -p "❓ Would you like to initialize a new control-plane here? [y/N]: " CONFIRM
    if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
        echo "❌ Aborting. No changes made."
        exit 0
    fi
    echo "🧭 Initializing new Kubernetes control-plane..."
    ./kub_install.sh 0
fi

# --- Operation choice ---
echo
echo "-----------------------------------------------"
echo "⚙️  Cluster Node Management"
echo "-----------------------------------------------"
echo "1) Add worker node(s)"
echo "2) Remove worker node(s)"
echo "3) Reset current node (destructive)"
echo "4) Show cluster status only"
read -p "Choose an option [1-4]: " choice
echo

case "$choice" in
1)
    # Temporarily restore space splitting for clean array parsing
    OLD_IFS="$IFS"
    IFS=' '
    read -p "Enter worker IP(s) to add (space-separated): " -a WORKER_IPS
    IFS="$OLD_IFS"

    # Filter out any empty entries caused by trailing spaces
    CLEAN_WORKERS=()
    for IP in "${WORKER_IPS[@]}"; do
        [[ -n "$IP" ]] && CLEAN_WORKERS+=("$IP")
    done

    if [ ${#CLEAN_WORKERS[@]} -eq 0 ]; then
        echo "⏭️  No worker nodes provided. Skipping."
    else
        echo "📋 Nodes to be added: ${CLEAN_WORKERS[*]}"
        for IP in "${CLEAN_WORKERS[@]}"; do
            echo
            read -p "❓ Are you sure you want to add node with IP ${IP}? [y/N]: " CONFIRM
            if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
                echo "⏭️  Skipping addition of ${IP}."
                continue
            fi

            echo "🔧 Checking $IP..."
            if worker_exists "$IP"; then
                echo "✔️  Node at $IP already part of cluster. Skipping."
            else
                echo "🧱 Adding new worker node at $IP..."
                ./kub_addNode.sh "$IP"
            fi
        done
    fi
    ;;
2)
    # Temporarily restore space splitting for clean array parsing
    OLD_IFS="$IFS"
    IFS=' '
    read -p "Enter worker IP(s) to remove (space-separated): " -a REMOVE_IPS
    IFS="$OLD_IFS"

    # Filter out any empty entries caused by extra spaces
    CLEAN_IPS=()
    for IP in "${REMOVE_IPS[@]}"; do
        [[ -n "$IP" ]] && CLEAN_IPS+=("$IP")
    done

    if [ ${#CLEAN_IPS[@]} -eq 0 ]; then
        echo "⏭️  No valid worker nodes provided. Skipping."
    else
        echo "📋 Nodes to be removed: ${CLEAN_IPS[*]}"
        for IP in "${CLEAN_IPS[@]}"; do
            echo
            remove_worker "$IP"
            echo
        done
    fi
    ;;
3)
    read -p "💥❓ WARNING: Are you sure you want to RESET the current node? [y/N]: " CONFIRM
    if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
        echo "❌ Aborting. No changes made."
        exit 0
    fi
    echo "💥💥💥"
    ./kub_reset.sh
    ;;
4)
    if kubectl get nodes >/dev/null 2>&1; then
        echo "📊 Gathering cluster status..."
        echo
        kubectl get nodes -o wide || echo "(no active nodes)"
        echo
        kubectl get pods -A -o wide 2>/dev/null | grep -v "kube-system" || { echo "(no user pods yet)"; true; }
        echo
        kubectl get svc -A || echo "(no services)"
        #echo 
        #echo "✅ Cluster status displayed."
    else
        echo "❌ Kubernetes not installed!"
    fi

    exit 0
    ;;
*)
    echo "❌ Invalid option."
    exit 1
    ;;
esac

# --- Cluster summary ---
if kubectl get nodes >/dev/null 2>&1; then
    echo
    echo "-----------------------------------------------"
    echo "📊 Cluster Summary"
    echo "-----------------------------------------------"
    kubectl get nodes -o wide || echo "(no active nodes)"
    echo
    kubectl get pods -A -o wide | grep -v "kube-system" || echo "(no user pods yet)"
    echo
    kubectl get svc -A
fi

#echo
#echo "🎉 Cluster ready!"
#echo "To interact with it:"
#echo "  kubectl get nodes"
#echo "  kubectl get pods -A"
#echo "  kubectl get svc -A"
#echo