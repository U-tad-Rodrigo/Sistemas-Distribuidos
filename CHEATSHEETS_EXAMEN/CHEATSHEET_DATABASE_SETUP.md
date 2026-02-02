# 🗄️ CHEATSHEET: Database Setup RDS MySQL - MiniTwitter
## Práctica 3 - Sistemas Distribuidos | Examen Final

---

## 📊 **SCRIPT SQL COMPLETO**

```sql
-- ============================================
-- BASE DE DATOS MINITWITTER
-- Rodrigo - Práctica 3
-- ============================================

CREATE DATABASE IF NOT EXISTS twitter CHARACTER SET utf8mb4;
USE twitter;

-- ============================================
-- TABLA 1: users (para login)
-- ============================================
CREATE TABLE users (
    username VARCHAR(50) PRIMARY KEY,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(100)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================
-- TABLA 2: posts (SIN id - importante)
-- ============================================
CREATE TABLE posts (
    user VARCHAR(50) NOT NULL,
    comment TEXT NOT NULL,
    attachment VARCHAR(500)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================
-- DATOS DE PRUEBA
-- ============================================
INSERT INTO users VALUES 
('admin', 'admin123', 'admin@minitwitter.com'),
('rodrigo', 'password', 'rodrigo@minitwitter.com');

INSERT INTO posts VALUES 
('admin', 'Primer post del sistema', NULL),
('rodrigo', 'Post con video', 'https://microtwitter.s3.us-east-1.amazonaws.com/video.mp4');

-- ============================================
-- CONSULTAS ÚTILES
-- ============================================

-- Ver todos los posts
SELECT * FROM posts;

-- Ver todos los usuarios
SELECT * FROM users;

-- Contar posts
SELECT COUNT(*) FROM posts;

-- Posts de un usuario
SELECT * FROM posts WHERE user = 'admin';

-- Borrar todos los posts
TRUNCATE TABLE posts;
-- o
DELETE FROM posts;

-- Borrar un post específico
DELETE FROM posts WHERE user = 'admin' AND comment LIKE '%texto%';
```

---

## ⚙️ **CONFIGURACIÓN RDS**

### Crear instancia RDS
```
Engine: MySQL 8.0
Template: Free tier
DB identifier: twitter-db
Master username: admin
Master password: password (elegir una segura)
DB instance: db.t3.micro
Storage: 20 GB
Public access: Yes
VPC security group: Create new (twitter-sg)
Initial database: twitter
```

### Security Group
```
Type: MySQL/Aurora
Port: 3306
Source: 0.0.0.0/0
Description: MySQL access
```

### Endpoint
Guardar el endpoint que aparece en RDS Console:
```
twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com
```

---

## 🔌 **CONEXIÓN DESDE LAMBDA**

```python
import pymysql

def get_connection():
    return pymysql.connect(
        host='twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com',
        user='admin',
        passwd='password',
        db='twitter',
        port=3306,
        connect_timeout=10
    )
```

---

## 📝 **OPERACIONES CRUD**

### CREATE - Insertar
```python
conn = get_connection()
with conn.cursor() as cur:
    cur.execute(
        "INSERT INTO posts (user, comment, attachment) VALUES (%s, %s, %s)",
        (user, comment, attachment)
    )
    conn.commit()
conn.close()
```

### READ - Leer
```python
conn = get_connection()
with conn.cursor() as cur:
    cur.execute("SELECT * FROM posts")
    rows = cur.fetchall()
    # rows es una tupla: (user, comment, attachment)
conn.close()
```

### UPDATE - Actualizar
```python
conn = get_connection()
with conn.cursor() as cur:
    cur.execute(
        "UPDATE posts SET comment = %s WHERE user = %s",
        (new_comment, username)
    )
    conn.commit()
conn.close()
```

### DELETE - Eliminar
```python
conn = get_connection()
with conn.cursor() as cur:
    cur.execute("DELETE FROM posts WHERE user = %s", (username,))
    conn.commit()
conn.close()
```

---

## 🛡️ **SEGURIDAD: SQL INJECTION**

### ❌ INCORRECTO (VULNERABLE)
```python
# NUNCA HAGAS ESTO
query = "SELECT * FROM users WHERE username = '" + username + "'"
cur.execute(query)  # ¡PELIGRO!
```

**Ataque posible:** `admin' OR '1'='1` → Acceso no autorizado

### ✅ CORRECTO (SEGURO)
```python
# SIEMPRE USA PREPARED STATEMENTS
query = "SELECT * FROM users WHERE username = %s"
cur.execute(query, (username,))  # ✅ SEGURO
```

**Regla de oro:** NUNCA concatenar strings, SIEMPRE usar %s con tuplas

---

## 🔄 **MANEJO DE ERRORES**

```python
import pymysql

conn = None
try:
    conn = get_connection()
    with conn.cursor() as cur:
        cur.execute("INSERT INTO posts VALUES (%s, %s, %s)", (u, c, a))
        conn.commit()
        
except pymysql.MySQLError as e:
    print(f"Error MySQL: {e}")
    if conn:
        conn.rollback()  # Revertir cambios
        
finally:
    if conn:
        conn.close()  # SIEMPRE cerrar
```

---

## 📊 **ESTRUCTURA DE LA TABLA POSTS**

### ⚠️ IMPORTANTE: Sin campo 'id'

El código del profesor hace:
```python
cur.execute("INSERT INTO posts VALUES ('user', 'comment', 'url')")
```

Si tu tabla tiene `id AUTO_INCREMENT`, esto FALLARÁ.

**Tabla correcta:**
```sql
CREATE TABLE posts (
    user VARCHAR(50),      -- Sin id
    comment TEXT,
    attachment VARCHAR(500)
);
```

**Tabla incorrecta:**
```sql
CREATE TABLE posts (
    id INT AUTO_INCREMENT PRIMARY KEY,  -- ❌ NO
    user VARCHAR(50),
    comment TEXT,
    attachment VARCHAR(500)
);
```

---

## 🎓 **PREGUNTAS DE EXAMEN**

### 1. ¿Qué es RDS?
Relational Database Service - Base de datos gestionada por AWS con backups automáticos, parches y alta disponibilidad.

### 2. ¿Por qué usar prepared statements?
- Previenen SQL injection
- Mejoran performance (query pre-compilado)
- Separan código SQL de datos

### 3. ¿Diferencia entre fetchone() y fetchall()?
- `fetchone()`: Devuelve UNA tupla o None
- `fetchall()`: Devuelve LISTA de tuplas

### 4. ¿Qué es TRUNCATE vs DELETE?
- `TRUNCATE`: Borra todo, más rápido, resetea auto_increment
- `DELETE`: Borra fila por fila, más lento, puede tener WHERE

### 5. ¿Para qué sirve conn.commit()?
Confirma los cambios permanentemente en la base de datos. Sin commit(), los INSERT/UPDATE/DELETE no se guardan.

### 6. ¿Qué hace conn.rollback()?
Revierte todos los cambios no confirmados. Útil si hay error en una transacción.

---

## 🔧 **COMANDOS MYSQL ÚTILES**

```sql
-- Ver tablas
SHOW TABLES;

-- Ver estructura de tabla
DESCRIBE posts;

-- Ver usuarios MySQL
SELECT user, host FROM mysql.user;

-- Ver último error
SHOW ERRORS;

-- Ver tamaño de tablas
SELECT 
    table_name,
    table_rows,
    data_length,
    index_length
FROM information_schema.tables 
WHERE table_schema = 'twitter';
```

---

## 📌 **CONECTARSE A RDS**

### Desde MySQL Workbench
```
Hostname: twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com
Port: 3306
Username: admin
Password: password
Default Schema: twitter
```

### Desde Terminal
```bash
mysql -h twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com \
      -u admin \
      -p \
      twitter
```

### Desde Python (testing local)
```python
import pymysql

conn = pymysql.connect(
    host='twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com',
    user='admin',
    passwd='password',
    db='twitter'
)

print("✅ Conexión exitosa")
conn.close()
```

---

## 🐛 **ERRORES COMUNES**

| Error | Causa | Solución |
|-------|-------|----------|
| `Can't connect to MySQL server` | Security Group bloqueado | Abrir puerto 3306 |
| `Access denied for user` | Usuario/contraseña incorrectos | Verificar credenciales |
| `Unknown database 'twitter'` | Base de datos no existe | Crear con CREATE DATABASE |
| `Table 'posts' doesn't exist` | Tabla no creada | Ejecutar CREATE TABLE |
| `Column count doesn't match` | Tabla tiene más/menos columnas | Verificar estructura |
| `Duplicate entry` | Clave primaria duplicada | Username ya existe |

---

## ✅ **CHECKLIST DATABASE**

- [ ] RDS creada en AWS
- [ ] Security Group con puerto 3306 abierto
- [ ] Base de datos `twitter` creada
- [ ] Tabla `users` creada con username como PRIMARY KEY
- [ ] Tabla `posts` creada SIN campo id
- [ ] Usuarios de prueba insertados
- [ ] Posts de prueba insertados
- [ ] Conexión desde MySQL Workbench probada
- [ ] Endpoint guardado para usar en Lambda

---

## 💡 **TIPS FINALES**

1. **Siempre cerrar conexiones:** Usar `finally` block
2. **Usar prepared statements:** Nunca concatenar SQL
3. **Commit después de INSERT/UPDATE/DELETE**
4. **Verificar Security Group:** Principal causa de errores de conexión
5. **Tabla posts sin id:** El código del profesor no lo usa

---

**¡Éxito en el examen! 🎯**
# 📚 CHEATSHEET: AWS Lambda Functions - MiniTwitter
## Práctica 3 - Sistemas Distribuidos | Examen Final

---

## 🎯 **4 LAMBDAS DEL PROYECTO**

### **1. twitterS3Presign (firmaS3)** ⭐ CLAVE
Genera credenciales AWS firmadas para subir archivos a S3 desde el navegador.

**¿Por qué es necesaria?**
- El navegador NO puede subir archivos a S3 sin credenciales AWS
- Lambda genera credenciales temporales (válidas 1 hora)
- El navegador las usa para subir directamente a S3
- Sin límite de tamaño (hasta 5GB en S3)

**Código:**
```python
import json, base64, datetime, hashlib, hmac, boto3

bucket = "microtwitter"
region = 'us-east-1'
service = 's3'

def sign(key, msg):
    return hmac.new(key, msg.encode('utf-8'), hashlib.sha256).digest()

def getSignatureKey(key, dateStamp, regionName, serviceName):
    kDate = sign(('AWS4' + key).encode('utf-8'), dateStamp)
    kRegion = sign(kDate, regionName)
    kService = sign(kRegion, serviceName)
    kSigning = sign(kService, 'aws4_request')
    return kSigning

def lambda_handler(event, context):
    session = boto3.Session()
    creds = session.get_credentials().get_frozen_credentials()

    t = datetime.datetime.utcnow()
    amzDate = t.strftime('%Y%m%dT%H%M%SZ')
    dateStamp = t.strftime('%Y%m%d')

    policy_structure = {
        "expiration": "2025-12-30T12:00:00.000Z",
        "conditions": [
            {"bucket": bucket},
            ["starts-with", "$key", ""],
            {"success_action_status": "201"},
            {"x-amz-credential": creds.access_key + "/" + dateStamp + "/" + region + "/s3/aws4_request"},
            {"x-amz-algorithm": "AWS4-HMAC-SHA256"},
            {"x-amz-date": amzDate},
            {"x-amz-security-token": creds.token}
        ]
    }

    policy_json = json.dumps(policy_structure)
    stringToSign = base64.b64encode(policy_json.encode("utf-8"))
    signing_key = getSignatureKey(creds.secret_key, dateStamp, region, service)
    signature = hmac.new(signing_key, stringToSign, hashlib.sha256).hexdigest()

    return {
        'statusCode': 200,
        'body': json.dumps({
            'stringSigned': signature,
            'stringToSign': stringToSign.decode('utf-8'),
            'xAmzCredential': creds.access_key + "/" + dateStamp + "/" + region + "/s3/aws4_request",
            'dateStamp': dateStamp,
            'amzDate': amzDate,
            'securityToken': creds.token
        })
    }
```

**Configuración:**
- Runtime: Python 3.12
- Permisos IAM: **AmazonS3FullAccess**
- Function URL: NONE (CORS en configuración)
- Variables entorno: `BUCKET_NAME=microtwitter`

**Devuelve:**
```json
{
  "stringSigned": "abc123...",
  "stringToSign": "eyJleH...",
  "xAmzCredential": "AKIAIOSFODNN7EXAMPLE/20251216/us-east-1/s3/aws4_request",
  "dateStamp": "20251216",
  "amzDate": "20251216T220000Z",
  "securityToken": "IQoJb3JpZ2..."
}
```

---

### **2. twitterPost**
Guarda posts en la base de datos RDS MySQL.

**Código:**
```python
import logging, pymysql, json, os

rds_host = "twitter.cjxsiharbt69.us-east-1.rds.amazonaws.com"
username = "admin"
password = "password"
dbname = "twitter"
bucket = "microtwitter"
bucketUrl = f"https://{bucket}.s3.us-east-1.amazonaws.com/"

logger = logging.getLogger()
logger.setLevel(logging.INFO)

def lambda_handler(event, context):
    logger.info(json.dumps(event))

    params = event.get("queryStringParameters") or {}
    user = params.get("user", "anonymous")
    comment = params.get("comment", "")
    attachment_file = params.get("attachment", "")
    
    attachment = bucketUrl + attachment_file if attachment_file else ""

    conn = None
    try:
        conn = pymysql.connect(
            host=rds_host, user=username, passwd=password, 
            db=dbname, connect_timeout=10, port=3306
        )
        
        with conn.cursor() as cur:
            cur.execute(
                "INSERT INTO posts (user, comment, attachment) VALUES (%s, %s, %s)",
                (user, comment, attachment)
            )
            conn.commit()
            
    except pymysql.MySQLError as e:
        logger.error(f"Error MySQL: {e}")
        return {
            'statusCode': 500,
            'body': json.dumps({'status': 'ERROR', 'message': str(e)})
        }
    finally:
        if conn:
            conn.close()

    return {
        'statusCode': 200,
        'body': json.dumps({'status': 'OK'})
    }
```

**Configuración:**
- Incluir pymysql en ZIP
- Timeout: 30 segundos
- Function URL con CORS

---

### **3. twitterRead**
Lee todos los posts de la base de datos.

**Código:**
```python
import pymysql, json

rds_host = "twitter.cjxsiharbt69.us-east-1.rds.amazonaws.com"
username = "admin"
password = "password"
dbname = "twitter"

def lambda_handler(event, context):
    commentList="{\"posts\": ["
    
    try:
        conn = pymysql.connect(
            host=rds_host, user=username, passwd=password, 
            db=dbname, connect_timeout=10, port=3306
        )
        
        with conn.cursor() as cur:
            cur.execute("select * from posts")
            rows = cur.fetchall()
            
            for row in rows:
                commentList+="{\"user\": \""+row[0]+"\", \"comment\": \""+row[1].replace("\n","\\n")+"\", \"attachment\": \""+row[2]+"\"},"
                    
            commentList=commentList[:-1]+"]}";
            
    except pymysql.MySQLError as e:
        print(e)
        
    finally:
        conn.close()
        
    return {
        'statusCode': 200,
        'body': commentList
    }
```

---

### **4. twitterAuth** (Avanzado)
Valida usuarios contra la base de datos.

**Código:**
```python
import pymysql, json

rds_host = "twitter.cjxsiharbt69.us-east-1.rds.amazonaws.com"
username = "admin"
password = "password"
dbname = "twitter"

def lambda_handler(event, context):
    params = event.get("queryStringParameters", {})
    user = params.get("username", "")
    pwd = params.get("password", "")
    
    try:
        conn = pymysql.connect(
            host=rds_host, user=username, passwd=password, 
            db=dbname, connect_timeout=10, port=3306
        )
        
        with conn.cursor() as cur:
            cur.execute("SELECT password FROM users WHERE username = %s", (user,))
            result = cur.fetchone()
        
        conn.close()
        
        if result and result[0] == pwd:
            return {
                'statusCode': 200,
                'body': json.dumps({'status': 'OK'})
            }
        else:
            return {
                'statusCode': 401,
                'body': json.dumps({'status': 'KO'})
            }
            
    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps({'status': 'ERROR'})
        }
```

---

## 🌐 **CORS en Lambda**

### ⚠️ IMPORTANTE: No duplicar headers CORS

**Si usas Function URL con CORS habilitado:**
- NO pongas headers CORS en el código
- AWS los añade automáticamente

**Si NO usas Function URL:**
- Añade en cada return:
```python
'headers': { 'Access-Control-Allow-Origin' : '*' }
```

**Error común:** `access-control-allow-origin: *, *` → Headers duplicados

---

## 🔧 **Configuración Function URL**

Para cada Lambda:
```
Configuration → Function URL → Create
Auth type: NONE
CORS: Enable
  - Allow origin: *
  - Allow methods: GET, POST, OPTIONS
  - Allow headers: *
```

---

## 📦 **Crear ZIP para Lambda**

```cmd
cd carpeta_lambda
powershell Compress-Archive -Path * -DestinationPath lambda.zip -Force
```

**Estructura correcta del ZIP:**
```
lambda_function.py  ← En la RAÍZ
pymysql/
  __init__.py
  connections.py
  ...
```

---

## 🎓 **PREGUNTAS DE EXAMEN**

### 1. ¿Para qué sirve twitterS3Presign?
Genera credenciales AWS temporales firmadas con AWS Signature V4 para permitir que el navegador suba archivos directamente a S3 sin exponer credenciales permanentes.

### 2. ¿Qué es AWS Signature V4?
Protocolo de autenticación de AWS que:
- Genera una política (policy) con condiciones
- La codifica en Base64
- La firma con HMAC-SHA256 usando la clave secreta
- Produce credenciales temporales válidas por tiempo limitado

### 3. ¿Por qué usar pymysql en lugar de mysql-connector?
- pymysql es Python puro (no requiere librerías C)
- Más ligero y compatible con Lambda
- Fácil de incluir en el ZIP

### 4. ¿Cuál es el flujo completo de publicar un post con video?

```
1. Usuario abre postComment.html
2. HTML pide credenciales a twitterS3Presign
3. Lambda genera firma AWS y devuelve credenciales
4. Usuario escribe post y selecciona video
5. HTML sube video a S3 con credenciales firmadas (POST directo)
6. HTML llama a twitterPost con URL del video
7. twitterPost guarda en RDS: (user, comment, url_video)
8. Usuario ve timeline con twitterRead
```

### 5. Diferencia entre firmaS3 y Pre-signed URLs de boto3

**Firma S3 (POST):** Para formularios HTML que suben archivos
**Pre-signed URL (PUT):** URL directa para subir con PUT request

Ambos hacen lo mismo (subir a S3 sin credenciales), pero con métodos HTTP diferentes.

---

## ✅ **CHECKLIST LAMBDA**

- [ ] **twitterS3Presign**: Genera firmas S3 (necesita permisos S3)
- [ ] **twitterPost**: Guarda posts en RDS (necesita pymysql)
- [ ] **twitterRead**: Lee posts de RDS (necesita pymysql)
- [ ] **twitterAuth**: Valida login (necesita pymysql)
- [ ] Todas tienen Function URL configurada
- [ ] CORS habilitado en Function URL (NO en código)
- [ ] Timeout 30 segundos mínimo
- [ ] pymysql incluido en el ZIP

---

## 🐛 **ERRORES COMUNES**

| Error | Causa | Solución |
|-------|-------|----------|
| `Unable to import module` | ZIP mal estructurado | lambda_function.py en raíz |
| `CORS: *, *` | Headers duplicados | Quitar del código, dejar en Function URL |
| `Can't connect MySQL` | Security Group | Abrir puerto 3306 desde 0.0.0.0/0 |
| `AccessDenied S3` | Sin permisos | IAM Role → AmazonS3FullAccess |
| `Timeout 3s` | Query lenta | Aumentar timeout a 30s |

---

**¡Éxito en el examen! 🚀**

