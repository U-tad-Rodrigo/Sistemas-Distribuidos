# ☁️ CHEATSHEET: AWS COMPLETO - Servicios Cloud
## Práctica 3 - Sistemas Distribuidos | Examen Final

---

## 📑 **ÍNDICE**

1. [S3 - Simple Storage Service](#s3)
2. [Lambda Functions](#lambda)
3. [API Gateway](#api-gateway)
4. [RDS - Relational Database Service](#rds)
5. [IAM - Identity and Access Management](#iam)
6. [CloudWatch](#cloudwatch)
7. [EC2 - Elastic Compute Cloud](#ec2)
8. [Amplify](#amplify)

---

## 📦 **S3 - SIMPLE STORAGE SERVICE**

### ¿Qué es S3?
Servicio de almacenamiento de objetos (archivos) en la nube.

### Crear un Bucket S3

#### Desde Consola AWS
```
1. AWS Console → S3 → Create bucket
2. Bucket name: microtwitter (debe ser único globalmente)
3. Region: us-east-1
4. Block all public access: DESMARCAR
5. Bucket Versioning: Disable
6. Tags: opcional
7. Create bucket
```

#### Configurar Permisos Públicos

**Bucket Policy (importante para acceso público):**
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Sid": "PublicReadGetObject",
            "Effect": "Allow",
            "Principal": "*",
            "Action": "s3:GetObject",
            "Resource": "arn:aws:s3:::microtwitter/*"
        }
    ]
}
```

**CORS Configuration:**
```json
[
    {
        "AllowedHeaders": ["*"],
        "AllowedMethods": ["GET", "PUT", "POST", "DELETE", "HEAD"],
        "AllowedOrigins": ["*"],
        "ExposeHeaders": ["ETag"],
        "MaxAgeSeconds": 3000
    }
]
```

### Subir Archivos a S3

#### Desde Python (Lambda o local)
```python
import boto3

s3_client = boto3.client('s3')

# Subir archivo
s3_client.upload_file(
    'local_file.mp4',
    'microtwitter',
    'uploads/video.mp4',
    ExtraArgs={'ContentType': 'video/mp4'}
)

# URL pública
url = f"https://microtwitter.s3.us-east-1.amazonaws.com/uploads/video.mp4"
```

#### Desde CLI AWS
```bash
aws s3 cp archivo.mp4 s3://microtwitter/uploads/archivo.mp4
aws s3 ls s3://microtwitter/
aws s3 rm s3://microtwitter/uploads/archivo.mp4
```

### Presigned URLs (URLs Firmadas)

**¿Por qué?** Subir archivos grandes directamente desde el frontend sin pasar por Lambda.

```python
import boto3
from datetime import datetime

s3_client = boto3.client('s3', region_name='us-east-1')

# Generar URL para SUBIR (PUT)
presigned_url = s3_client.generate_presigned_url(
    'put_object',
    Params={
        'Bucket': 'microtwitter',
        'Key': 'uploads/video.mp4',
        'ContentType': 'video/mp4'
    },
    ExpiresIn=300  # 5 minutos
)

# Generar URL para DESCARGAR (GET)
download_url = s3_client.generate_presigned_url(
    'get_object',
    Params={
        'Bucket': 'microtwitter',
        'Key': 'uploads/video.mp4'
    },
    ExpiresIn=3600  # 1 hora
)
```

### Usar Presigned URL desde Frontend
```javascript
// Obtener presigned URL del backend
const response = await fetch('https://api.../upload', {
    method: 'POST',
    body: JSON.stringify({
        filename: 'video.mp4',
        filetype: 'video/mp4'
    })
});

const { uploadURL, fileURL } = await response.json();

// Subir archivo directamente a S3
const file = document.getElementById('fileInput').files[0];
await fetch(uploadURL, {
    method: 'PUT',
    body: file,
    headers: {
        'Content-Type': file.type
    }
});

console.log('Archivo disponible en:', fileURL);
```

### Estructura Recomendada
```
microtwitter/
├── uploads/
│   ├── 20241217_143022_video.mp4
│   ├── 20241217_143055_image.jpg
│   └── 20241217_143120_audio.mp3
├── avatars/
│   ├── admin.jpg
│   └── rodrigo.jpg
└── static/
    ├── logo.png
    └── background.jpg
```

---

## ⚡ **LAMBDA FUNCTIONS**

### ¿Qué es Lambda?
Servicio serverless para ejecutar código sin gestionar servidores. Pagas solo por tiempo de ejecución.

### Crear una Lambda

```
AWS Console → Lambda → Create function
Function name: loginLambda
Runtime: Python 3.12
Architecture: x86_64
Permissions: Create new role
```

### Configuración Básica

**Timeout:** 30 segundos (Configuration → General)
**Memory:** 128 MB (suficiente para la mayoría)
**Environment variables:** DB_HOST, DB_USER, DB_PASSWORD, etc.

### Lambda: Login Completo

```python
import json
import pymysql
import os

def lambda_handler(event, context):
    # Variables de entorno
    DB_HOST = os.environ.get('DB_HOST')
    DB_USER = os.environ.get('DB_USER')
    DB_PASSWORD = os.environ.get('DB_PASSWORD')
    DB_NAME = os.environ.get('DB_NAME', 'twitter')
    
    try:
        # Parsear body
        body = json.loads(event.get('body', '{}'))
        username = body.get('username')
        password = body.get('password')
        
        # Validar inputs
        if not username or not password:
            return response(400, {'message': 'Campos requeridos'})
        
        # Conectar a DB
        conn = pymysql.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            cursorclass=pymysql.cursors.DictCursor
        )
        
        with conn:
            with conn.cursor() as cursor:
                sql = "SELECT username, email FROM users WHERE username=%s AND password=%s"
                cursor.execute(sql, (username, password))
                user = cursor.fetchone()
                
                if user:
                    return response(200, {
                        'message': 'Login exitoso',
                        'user': user
                    })
                else:
                    return response(401, {'message': 'Credenciales inválidas'})
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return response(500, {'message': 'Error del servidor', 'error': str(e)})

def response(status_code, body):
    return {
        'statusCode': status_code,
        'headers': {
            'Access-Control-Allow-Origin': '*',
            'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
            'Content-Type': 'application/json'
        },
        'body': json.dumps(body)
    }
```

### Lambda Layers (pymysql)

**¿Por qué?** Lambda no incluye pymysql por defecto.

#### Crear Layer localmente
```bash
# Windows (CMD)
mkdir python
pip install pymysql -t python
tar -a -c -f pymysql-layer.zip python

# Linux/Mac
mkdir python
pip install pymysql -t python
zip -r pymysql-layer.zip python
```

#### Subir a AWS
```
Lambda → Layers → Create layer
Name: pymysql-layer
Upload: pymysql-layer.zip
Compatible runtimes: Python 3.12
```

#### Añadir a Lambda
```
Lambda function → Layers → Add a layer
Custom layers → pymysql-layer → Version 1
```

### Trigger: API Gateway

```
Lambda → Add trigger → API Gateway
API: Create new API
API type: HTTP API
Security: Open
```

---

## 🌐 **API GATEWAY**

### ¿Qué es API Gateway?
Servicio para crear, publicar y gestionar APIs RESTful que se conectan a Lambdas.

### Crear API REST

```
API Gateway → Create API → REST API
API name: MiniTwitterAPI
Endpoint Type: Regional
```

### Crear Recursos y Métodos

#### Estructura Recomendada
```
/
├── /login [POST]
├── /register [POST]
├── /posts
│   ├── GET (obtener todos)
│   └── POST (crear nuevo)
├── /upload [POST]
└── /user
    └── /{username} [GET]
```

#### Crear Recurso
```
Actions → Create Resource
Resource Name: login
Resource Path: /login
Enable CORS: ✓
```

#### Crear Método
```
Actions → Create Method → POST
Integration type: Lambda Function
Lambda Region: us-east-1
Lambda Function: loginLambda
Use Lambda Proxy integration: ✓
```

### Habilitar CORS

**Método 1: Automático**
```
Actions → Enable CORS
Access-Control-Allow-Origin: '*'
Access-Control-Allow-Headers: 'Content-Type,X-Amz-Date,Authorization'
Access-Control-Allow-Methods: 'GET,POST,PUT,DELETE,OPTIONS'
```

**Método 2: Manual (en Lambda)**
```python
headers = {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type, Authorization'
}
```

### Deploy API

```
Actions → Deploy API
Deployment stage: [New Stage]
Stage name: prod
```

### URL Final
```
https://abc123xyz.execute-api.us-east-1.amazonaws.com/prod/login
```

### Probar API

#### Con curl
```bash
curl -X POST https://abc123xyz.execute-api.us-east-1.amazonaws.com/prod/login \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"admin\",\"password\":\"admin123\"}"
```

#### Con Postman
```
Method: POST
URL: https://abc123xyz.execute-api.us-east-1.amazonaws.com/prod/login
Headers:
  Content-Type: application/json
Body (raw):
{
    "username": "admin",
    "password": "admin123"
}
```

#### Desde JavaScript
```javascript
const login = async (username, password) => {
    const response = await fetch('https://abc123xyz.../prod/login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ username, password })
    });
    
    const data = await response.json();
    return data;
};
```

---

## 🗄️ **RDS - RELATIONAL DATABASE SERVICE**

### ¿Qué es RDS?
Base de datos relacional gestionada (MySQL, PostgreSQL, etc.) en la nube.

### Crear Instancia RDS MySQL

```
RDS → Create database
Engine: MySQL 8.0
Template: Free tier
DB instance identifier: twitter-db
Master username: admin
Master password: Password123!
DB instance class: db.t3.micro
Storage: 20 GB gp2
Public access: Yes
VPC security group: Create new (twitter-sg)
Initial database name: twitter
```

### Security Group

```
Type: MySQL/Aurora
Protocol: TCP
Port: 3306
Source: 0.0.0.0/0 (o tu IP específica)
Description: MySQL access
```

### Endpoint

Copiar de RDS Console:
```
twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com
```

### Conectar desde Python

```python
import pymysql

connection = pymysql.connect(
    host='twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com',
    user='admin',
    password='Password123!',
    database='twitter',
    port=3306
)

with connection:
    with connection.cursor() as cursor:
        cursor.execute("SELECT * FROM users")
        result = cursor.fetchall()
        print(result)
```

### Conectar con MySQL Workbench

```
Connection Name: Twitter DB
Hostname: twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com
Port: 3306
Username: admin
Password: Password123!
Default Schema: twitter
```

---

## 🔐 **IAM - IDENTITY AND ACCESS MANAGEMENT**

### ¿Qué es IAM?
Gestión de permisos y accesos a servicios AWS.

### Roles para Lambda

#### Permisos Básicos (siempre necesario)
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "logs:CreateLogGroup",
                "logs:CreateLogStream",
                "logs:PutLogEvents"
            ],
            "Resource": "arn:aws:logs:*:*:*"
        }
    ]
}
```

#### Permisos S3 (para upload/download)
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "s3:PutObject",
                "s3:GetObject",
                "s3:DeleteObject",
                "s3:ListBucket"
            ],
            "Resource": [
                "arn:aws:s3:::microtwitter",
                "arn:aws:s3:::microtwitter/*"
            ]
        }
    ]
}
```

#### Permisos RDS (para Lambda)
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "rds:DescribeDBInstances",
                "rds:DescribeDBClusters"
            ],
            "Resource": "*"
        }
    ]
}
```

### Crear Role para Lambda

```
IAM → Roles → Create role
Trusted entity: AWS service → Lambda
Permissions policies:
  - AWSLambdaBasicExecutionRole
  - Crear política custom para S3
  - Crear política custom para RDS
Role name: lambda-execution-role
```

### Asignar Role a Lambda

```
Lambda → Configuration → Permissions
Execution role → Edit
Existing role: lambda-execution-role
```

---

## 📊 **CLOUDWATCH**

### ¿Qué es CloudWatch?
Servicio de monitoreo y logging de AWS.

### Log Groups

Cada Lambda crea automáticamente:
```
/aws/lambda/loginLambda
/aws/lambda/getPostsLambda
/aws/lambda/addPostLambda
```

### Ver Logs

```
CloudWatch → Logs → Log groups → /aws/lambda/loginLambda
```

### Logging desde Lambda

```python
import logging

logger = logging.getLogger()
logger.setLevel(logging.INFO)

def lambda_handler(event, context):
    logger.info(f"Event recibido: {event}")
    logger.error(f"Error ocurrido: {error}")
    
    # También funciona:
    print("Este mensaje aparece en CloudWatch")
```

### Métricas Importantes

- **Invocations:** Número de veces que se ejecutó
- **Duration:** Tiempo de ejecución
- **Errors:** Errores ocurridos
- **Throttles:** Invocaciones limitadas

### Alarmas

```
CloudWatch → Alarms → Create alarm
Metric: Lambda → Errors
Condition: > 5 errores en 5 minutos
Action: Send notification (SNS)
```

---

## 💻 **EC2 - ELASTIC COMPUTE CLOUD**

### ¿Qué es EC2?
Servidores virtuales en la nube.

### Crear Instancia EC2

```
EC2 → Launch Instance
Name: MiniTwitter-Server
AMI: Amazon Linux 2023 (Free tier)
Instance type: t2.micro
Key pair: Create new (microtwitter-key.pem)
Security group:
  - SSH (22) desde tu IP
  - HTTP (80) desde 0.0.0.0/0
  - HTTPS (443) desde 0.0.0.0/0
Storage: 8 GB gp2
```

### Conectar por SSH

```bash
# Windows (con PuTTY o Git Bash)
chmod 400 microtwitter-key.pem
ssh -i microtwitter-key.pem ec2-user@ec2-xx-xxx-xxx-xx.compute-1.amazonaws.com
```

### Instalar Node.js en EC2

```bash
# Actualizar sistema
sudo yum update -y

# Instalar Node.js
curl -fsSL https://rpm.nodesource.com/setup_18.x | sudo bash -
sudo yum install -y nodejs

# Verificar
node --version
npm --version
```

### Desplegar Aplicación

```bash
# Clonar proyecto
git clone https://github.com/tu-repo/minitwitter.git
cd minitwitter

# Instalar dependencias
npm install

# Ejecutar con PM2 (mantener corriendo)
sudo npm install -g pm2
pm2 start app.js
pm2 startup
pm2 save
```

---

## 🚀 **AMPLIFY**

### ¿Qué es Amplify?
Servicio para desplegar aplicaciones web frontend desde Git.

### Desplegar Frontend

```
Amplify → New app → Host web app
From GitHub → Authorize
Repository: minitwitter-frontend
Branch: main
Build settings: (auto-detecta)
Environment variables:
  REACT_APP_API_URL = https://abc123.../prod
Deploy
```

### Build Settings (React)

```yaml
version: 1
frontend:
  phases:
    preBuild:
      commands:
        - npm install
    build:
      commands:
        - npm run build
  artifacts:
    baseDirectory: build
    files:
      - '**/*'
  cache:
    paths:
      - node_modules/**/*
```

### URL Final
```
https://main.d1a2b3c4d5e6f7.amplifyapp.com
```

---

## 🎯 **ARQUITECTURA COMPLETA**

```
┌─────────────────┐
│   FRONTEND      │
│   (Amplify)     │
│   React/HTML    │
└────────┬────────┘
         │
         │ HTTPS
         ▼
┌─────────────────┐
│  API GATEWAY    │
│  (REST API)     │
└────────┬────────┘
         │
         │ Invoke
         ▼
┌─────────────────┐       ┌──────────────┐
│  LAMBDA         │◄─────►│  S3 BUCKET   │
│  (Python)       │       │  (Files)     │
└────────┬────────┘       └──────────────┘
         │
         │ Query
         ▼
┌─────────────────┐
│  RDS MySQL      │
│  (Database)     │
└─────────────────┘
```

---

## 📝 **FLUJO DE LOGIN COMPLETO**

### 1. Usuario ingresa credenciales
```javascript
// Frontend
const handleLogin = async () => {
    const response = await fetch(API_URL + '/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password })
    });
    const data = await response.json();
    if (data.username) {
        localStorage.setItem('user', data.username);
        window.location.href = '/home';
    }
};
```

### 2. API Gateway recibe request
```
POST /prod/login
Headers: Content-Type: application/json
Body: {"username":"admin","password":"admin123"}
```

### 3. Lambda procesa
```python
# Lambda loginLambda
body = json.loads(event['body'])
username = body['username']
password = body['password']

# Query a RDS
cursor.execute("SELECT * FROM users WHERE username=%s AND password=%s", 
               (username, password))
user = cursor.fetchone()

# Retornar respuesta
return response(200, {'username': user['username']})
```

### 4. RDS valida credenciales
```sql
SELECT username, email FROM users 
WHERE username='admin' AND password='admin123';
```

### 5. Respuesta al Frontend
```json
{
    "message": "Login exitoso",
    "username": "admin",
    "email": "admin@minitwitter.com"
}
```

---

## 🔥 **COMANDOS AWS CLI**

### Configurar AWS CLI
```bash
aws configure
AWS Access Key ID: tu_access_key
AWS Secret Access Key: tu_secret_key
Default region: us-east-1
Default output format: json
```

### S3 Commands
```bash
# Listar buckets
aws s3 ls

# Listar archivos en bucket
aws s3 ls s3://microtwitter/

# Subir archivo
aws s3 cp video.mp4 s3://microtwitter/uploads/

# Descargar archivo
aws s3 cp s3://microtwitter/uploads/video.mp4 ./

# Eliminar archivo
aws s3 rm s3://microtwitter/uploads/video.mp4

# Sincronizar directorio
aws s3 sync ./build s3://microtwitter/
```

### Lambda Commands
```bash
# Listar funciones
aws lambda list-functions

# Invocar función
aws lambda invoke --function-name loginLambda output.json

# Ver logs
aws logs tail /aws/lambda/loginLambda --follow
```

### RDS Commands
```bash
# Listar instancias
aws rds describe-db-instances

# Ver endpoint
aws rds describe-db-instances --db-instance-identifier twitter-db \
  --query 'DBInstances[0].Endpoint.Address'
```

---

## 🎓 **TIPS PARA EL EXAMEN**

### ✅ Checklist Pre-Examen

- [ ] **S3:** Bucket creado, política pública, CORS configurado
- [ ] **RDS:** Instancia creada, accesible, BD inicializada
- [ ] **Lambda:** Funciones creadas, variables de entorno, layers añadidos
- [ ] **API Gateway:** API creada, CORS habilitado, deployed
- [ ] **IAM:** Roles configurados con permisos correctos
- [ ] **Security Groups:** Puertos abiertos (3306, 443, 80)
- [ ] **Código:** Testeado localmente y en AWS
- [ ] **URLs:** Guardadas y accesibles

### ⚠️ Errores Comunes

1. **CORS no habilitado** → Error en frontend
2. **Security Group mal configurado** → Timeout en RDS
3. **Variables de entorno faltantes** → Error 500 en Lambda
4. **Layer pymysql no añadido** → Import error
5. **S3 no público** → 403 Forbidden en archivos
6. **Timeout muy corto** → Lambda termina antes de tiempo
7. **IAM sin permisos** → Access denied

### 🚨 Solución Rápida de Problemas

**Lambda no conecta a RDS:**
```
1. Verificar Security Group de RDS permite puerto 3306
2. Verificar endpoint correcto en variables de entorno
3. Verificar usuario/password correctos
4. Check CloudWatch logs para error específico
```

**CORS Error:**
```
1. API Gateway → Enable CORS
2. Añadir headers en Lambda response
3. Deploy API después de cambios
4. Clear browser cache
```

**500 Internal Server Error:**
```
1. Check CloudWatch logs de la Lambda
2. Verificar variables de entorno
3. Verificar layer pymysql añadido
4. Test Lambda directamente (no por API)
```

---

## 📚 **RECURSOS ADICIONALES**

### Documentación Oficial
- [AWS Lambda Docs](https://docs.aws.amazon.com/lambda/)
- [S3 Developer Guide](https://docs.aws.amazon.com/s3/)
- [API Gateway Docs](https://docs.aws.amazon.com/apigateway/)
- [RDS MySQL Docs](https://docs.aws.amazon.com/rds/)

### Free Tier Limits
- **Lambda:** 1M requests/month, 400,000 GB-seconds
- **S3:** 5 GB storage, 20,000 GET, 2,000 PUT
- **RDS:** 750 hours db.t2.micro/month
- **API Gateway:** 1M API calls/month

---

**¡Éxito en el examen! 🚀**

