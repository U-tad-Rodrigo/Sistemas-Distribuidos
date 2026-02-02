# ⚡ CHEATSHEET: AWS Lambda Functions - MiniTwitter
## Práctica 3 - Sistemas Distribuidos | Examen Final

---

## 📋 **ÍNDICE DE FUNCIONES LAMBDA**

1. **loginLambda** - Autenticación de usuarios
2. **getPostsLambda** - Obtener todos los posts
3. **addPostLambda** - Crear nuevo post
4. **getPresignedURLLambda** - Subir archivos a S3
5. **registerUserLambda** - Registrar nuevo usuario

---

## 🔐 **1. LOGIN LAMBDA**

### Configuración
```
Function name: loginLambda
Runtime: Python 3.12
Architecture: x86_64
Timeout: 30 seconds
Memory: 128 MB
```

### Código Completo
```python
import json
import pymysql
import os

def lambda_handler(event, context):
    """
    Lambda para autenticación de usuarios
    Método: POST
    Body: {"username": "admin", "password": "admin123"}
    """
    
    # Configuración de base de datos
    DB_HOST = os.environ.get('DB_HOST')
    DB_USER = os.environ.get('DB_USER', 'admin')
    DB_PASSWORD = os.environ.get('DB_PASSWORD')
    DB_NAME = os.environ.get('DB_NAME', 'twitter')
    
    try:
        # Parsear el body
        body = json.loads(event.get('body', '{}'))
        username = body.get('username')
        password = body.get('password')
        
        if not username or not password:
            return {
                'statusCode': 400,
                'headers': {
                    'Access-Control-Allow-Origin': '*',
                    'Access-Control-Allow-Methods': 'POST, OPTIONS',
                    'Content-Type': 'application/json'
                },
                'body': json.dumps({
                    'message': 'Username y password son requeridos'
                })
            }
        
        # Conectar a la base de datos
        connection = pymysql.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            cursorclass=pymysql.cursors.DictCursor
        )
        
        with connection:
            with connection.cursor() as cursor:
                # Consulta SQL
                sql = "SELECT username, email FROM users WHERE username = %s AND password = %s"
                cursor.execute(sql, (username, password))
                result = cursor.fetchone()
                
                if result:
                    return {
                        'statusCode': 200,
                        'headers': {
                            'Access-Control-Allow-Origin': '*',
                            'Access-Control-Allow-Methods': 'POST, OPTIONS',
                            'Content-Type': 'application/json'
                        },
                        'body': json.dumps({
                            'message': 'Login exitoso',
                            'username': result['username'],
                            'email': result['email']
                        })
                    }
                else:
                    return {
                        'statusCode': 401,
                        'headers': {
                            'Access-Control-Allow-Origin': '*',
                            'Access-Control-Allow-Methods': 'POST, OPTIONS',
                            'Content-Type': 'application/json'
                        },
                        'body': json.dumps({
                            'message': 'Credenciales inválidas'
                        })
                    }
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'headers': {
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Methods': 'POST, OPTIONS',
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'message': 'Error interno del servidor',
                'error': str(e)
            })
        }
```

### Variables de Entorno
```
DB_HOST = twitter-db.c3cisukkmohx.us-east-1.rds.amazonaws.com
DB_USER = admin
DB_PASSWORD = tu_password_seguro
DB_NAME = twitter
```

### Layer necesario: pymysql
Crear layer con pymysql para Python 3.12

---

## 📝 **2. GET POSTS LAMBDA**

### Código Completo
```python
import json
import pymysql
import os

def lambda_handler(event, context):
    """
    Lambda para obtener todos los posts
    Método: GET
    """
    
    DB_HOST = os.environ.get('DB_HOST')
    DB_USER = os.environ.get('DB_USER', 'admin')
    DB_PASSWORD = os.environ.get('DB_PASSWORD')
    DB_NAME = os.environ.get('DB_NAME', 'twitter')
    
    try:
        # Conectar a la base de datos
        connection = pymysql.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            cursorclass=pymysql.cursors.DictCursor
        )
        
        with connection:
            with connection.cursor() as cursor:
                # Obtener todos los posts ordenados
                sql = "SELECT user, comment, attachment FROM posts ORDER BY user DESC"
                cursor.execute(sql)
                results = cursor.fetchall()
                
                return {
                    'statusCode': 200,
                    'headers': {
                        'Access-Control-Allow-Origin': '*',
                        'Access-Control-Allow-Methods': 'GET, OPTIONS',
                        'Content-Type': 'application/json'
                    },
                    'body': json.dumps({
                        'posts': results,
                        'count': len(results)
                    })
                }
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'headers': {
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Methods': 'GET, OPTIONS',
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'message': 'Error al obtener posts',
                'error': str(e)
            })
        }
```

---

## ➕ **3. ADD POST LAMBDA**

### Código Completo
```python
import json
import pymysql
import os

def lambda_handler(event, context):
    """
    Lambda para agregar un nuevo post
    Método: POST
    Body: {"user": "admin", "comment": "Mi post", "attachment": "url_opcional"}
    """
    
    DB_HOST = os.environ.get('DB_HOST')
    DB_USER = os.environ.get('DB_USER', 'admin')
    DB_PASSWORD = os.environ.get('DB_PASSWORD')
    DB_NAME = os.environ.get('DB_NAME', 'twitter')
    
    try:
        # Parsear el body
        body = json.loads(event.get('body', '{}'))
        user = body.get('user')
        comment = body.get('comment')
        attachment = body.get('attachment', None)
        
        if not user or not comment:
            return {
                'statusCode': 400,
                'headers': {
                    'Access-Control-Allow-Origin': '*',
                    'Access-Control-Allow-Methods': 'POST, OPTIONS',
                    'Content-Type': 'application/json'
                },
                'body': json.dumps({
                    'message': 'Usuario y comentario son requeridos'
                })
            }
        
        # Conectar a la base de datos
        connection = pymysql.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            cursorclass=pymysql.cursors.DictCursor
        )
        
        with connection:
            with connection.cursor() as cursor:
                # Insertar post
                sql = "INSERT INTO posts (user, comment, attachment) VALUES (%s, %s, %s)"
                cursor.execute(sql, (user, comment, attachment))
                connection.commit()
                
                return {
                    'statusCode': 201,
                    'headers': {
                        'Access-Control-Allow-Origin': '*',
                        'Access-Control-Allow-Methods': 'POST, OPTIONS',
                        'Content-Type': 'application/json'
                    },
                    'body': json.dumps({
                        'message': 'Post creado exitosamente',
                        'post': {
                            'user': user,
                            'comment': comment,
                            'attachment': attachment
                        }
                    })
                }
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'headers': {
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Methods': 'POST, OPTIONS',
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'message': 'Error al crear post',
                'error': str(e)
            })
        }
```

---

## 📤 **4. GET PRESIGNED URL LAMBDA**

### Código Completo
```python
import json
import boto3
import os
from datetime import datetime

def lambda_handler(event, context):
    """
    Lambda para generar URL firmada para subir archivos a S3
    Método: POST
    Body: {"filename": "video.mp4", "filetype": "video/mp4"}
    """
    
    BUCKET_NAME = os.environ.get('BUCKET_NAME', 'microtwitter')
    
    try:
        # Parsear el body
        body = json.loads(event.get('body', '{}'))
        filename = body.get('filename')
        filetype = body.get('filetype')
        
        if not filename or not filetype:
            return {
                'statusCode': 400,
                'headers': {
                    'Access-Control-Allow-Origin': '*',
                    'Access-Control-Allow-Methods': 'POST, OPTIONS',
                    'Content-Type': 'application/json'
                },
                'body': json.dumps({
                    'message': 'filename y filetype son requeridos'
                })
            }
        
        # Generar nombre único
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        key = f"uploads/{timestamp}_{filename}"
        
        # Cliente S3
        s3_client = boto3.client('s3', region_name='us-east-1')
        
        # Generar presigned URL
        presigned_url = s3_client.generate_presigned_url(
            'put_object',
            Params={
                'Bucket': BUCKET_NAME,
                'Key': key,
                'ContentType': filetype
            },
            ExpiresIn=300  # 5 minutos
        )
        
        # URL pública del archivo
        public_url = f"https://{BUCKET_NAME}.s3.us-east-1.amazonaws.com/{key}"
        
        return {
            'statusCode': 200,
            'headers': {
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Methods': 'POST, OPTIONS',
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'uploadURL': presigned_url,
                'fileURL': public_url,
                'key': key
            })
        }
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'headers': {
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Methods': 'POST, OPTIONS',
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'message': 'Error al generar URL',
                'error': str(e)
            })
        }
```

### Variables de Entorno
```
BUCKET_NAME = microtwitter
```

### Permisos IAM necesarios
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "s3:PutObject",
                "s3:GetObject"
            ],
            "Resource": "arn:aws:s3:::microtwitter/*"
        }
    ]
}
```

---

## 👤 **5. REGISTER USER LAMBDA**

### Código Completo
```python
import json
import pymysql
import os
import re

def lambda_handler(event, context):
    """
    Lambda para registrar nuevos usuarios
    Método: POST
    Body: {"username": "nuevo", "password": "pass123", "email": "nuevo@email.com"}
    """
    
    DB_HOST = os.environ.get('DB_HOST')
    DB_USER = os.environ.get('DB_USER', 'admin')
    DB_PASSWORD = os.environ.get('DB_PASSWORD')
    DB_NAME = os.environ.get('DB_NAME', 'twitter')
    
    try:
        # Parsear el body
        body = json.loads(event.get('body', '{}'))
        username = body.get('username')
        password = body.get('password')
        email = body.get('email')
        
        # Validaciones
        if not username or not password or not email:
            return {
                'statusCode': 400,
                'headers': {
                    'Access-Control-Allow-Origin': '*',
                    'Access-Control-Allow-Methods': 'POST, OPTIONS',
                    'Content-Type': 'application/json'
                },
                'body': json.dumps({
                    'message': 'Todos los campos son requeridos'
                })
            }
        
        # Validar email
        email_regex = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
        if not re.match(email_regex, email):
            return {
                'statusCode': 400,
                'headers': {
                    'Access-Control-Allow-Origin': '*',
                    'Access-Control-Allow-Methods': 'POST, OPTIONS',
                    'Content-Type': 'application/json'
                },
                'body': json.dumps({
                    'message': 'Email inválido'
                })
            }
        
        # Conectar a la base de datos
        connection = pymysql.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            cursorclass=pymysql.cursors.DictCursor
        )
        
        with connection:
            with connection.cursor() as cursor:
                # Verificar si el usuario ya existe
                check_sql = "SELECT username FROM users WHERE username = %s"
                cursor.execute(check_sql, (username,))
                if cursor.fetchone():
                    return {
                        'statusCode': 409,
                        'headers': {
                            'Access-Control-Allow-Origin': '*',
                            'Access-Control-Allow-Methods': 'POST, OPTIONS',
                            'Content-Type': 'application/json'
                        },
                        'body': json.dumps({
                            'message': 'El usuario ya existe'
                        })
                    }
                
                # Insertar nuevo usuario
                insert_sql = "INSERT INTO users (username, password, email) VALUES (%s, %s, %s)"
                cursor.execute(insert_sql, (username, password, email))
                connection.commit()
                
                return {
                    'statusCode': 201,
                    'headers': {
                        'Access-Control-Allow-Origin': '*',
                        'Access-Control-Allow-Methods': 'POST, OPTIONS',
                        'Content-Type': 'application/json'
                    },
                    'body': json.dumps({
                        'message': 'Usuario registrado exitosamente',
                        'username': username,
                        'email': email
                    })
                }
    
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'headers': {
                'Access-Control-Allow-Origin': '*',
                'Access-Control-Allow-Methods': 'POST, OPTIONS',
                'Content-Type': 'application/json'
            },
            'body': json.dumps({
                'message': 'Error al registrar usuario',
                'error': str(e)
            })
        }
```

---

## 📦 **CREAR LAYER PYMYSQL**

### Paso 1: Estructura local
```
python/
  └── lib/
      └── python3.12/
          └── site-packages/
              └── pymysql/
```

### Paso 2: Instalar pymysql
```bash
mkdir python
pip install pymysql -t python/
zip -r pymysql-layer.zip python/
```

### Paso 3: Crear Layer en AWS
```
Name: pymysql-layer
Compatible runtimes: Python 3.12
Upload: pymysql-layer.zip
```

### Paso 4: Añadir Layer a Lambda
En cada Lambda → Layers → Add a layer → Custom layers → pymysql-layer

---

## 🔧 **CONFIGURACIÓN COMÚN PARA TODAS LAS LAMBDAS**

### Permisos IAM Role
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
        },
        {
            "Effect": "Allow",
            "Action": [
                "s3:*"
            ],
            "Resource": [
                "arn:aws:s3:::microtwitter",
                "arn:aws:s3:::microtwitter/*"
            ]
        },
        {
            "Effect": "Allow",
            "Action": [
                "rds:*"
            ],
            "Resource": "*"
        }
    ]
}
```

### Security Group
- Asegurarse que la Lambda tenga acceso al RDS
- Añadir regla de entrada en RDS Security Group:
  - Type: MySQL/Aurora
  - Port: 3306
  - Source: Lambda Security Group

---

## 🧪 **TESTS PARA CADA LAMBDA**

### Test Login
```json
{
  "body": "{\"username\": \"admin\", \"password\": \"admin123\"}"
}
```

### Test Get Posts
```json
{
  "httpMethod": "GET"
}
```

### Test Add Post
```json
{
  "body": "{\"user\": \"admin\", \"comment\": \"Test post desde Lambda\", \"attachment\": null}"
}
```

### Test Presigned URL
```json
{
  "body": "{\"filename\": \"test.mp4\", \"filetype\": \"video/mp4\"}"
}
```

### Test Register User
```json
{
  "body": "{\"username\": \"testuser\", \"password\": \"test123\", \"email\": \"test@example.com\"}"
}
```

---

## ⚡ **TIPS Y BUENAS PRÁCTICAS**

### 1. Headers CORS
Siempre incluir:
```python
'Access-Control-Allow-Origin': '*'
'Access-Control-Allow-Methods': 'GET, POST, OPTIONS'
'Content-Type': 'application/json'
```

### 2. Manejo de Errores
- Usar try-except siempre
- Retornar códigos HTTP apropiados
- Loggear errores con print() para CloudWatch

### 3. Variables de Entorno
- Nunca hardcodear credenciales
- Usar os.environ.get() con valores default
- Configurar en Lambda → Configuration → Environment variables

### 4. Timeout y Memory
- Login/Register: 30 seg, 128 MB
- Get Posts: 30 seg, 128 MB
- Add Post: 30 seg, 128 MB
- Presigned URL: 10 seg, 128 MB

### 5. Conexiones a Base de Datos
- Cerrar conexiones con `with connection:`
- Usar DictCursor para resultados más legibles
- Parametrizar queries para evitar SQL injection

---

## 🔗 **INTEGRACIÓN CON API GATEWAY**

### Crear API REST
1. API Gateway → Create API → REST API
2. Create Resource → /login, /posts, etc.
3. Create Method → POST, GET
4. Integration type: Lambda Function
5. Enable CORS
6. Deploy API → Stage: prod

### Endpoints resultantes
```
POST   https://api-id.execute-api.us-east-1.amazonaws.com/prod/login
GET    https://api-id.execute-api.us-east-1.amazonaws.com/prod/posts
POST   https://api-id.execute-api.us-east-1.amazonaws.com/prod/posts
POST   https://api-id.execute-api.us-east-1.amazonaws.com/prod/upload
POST   https://api-id.execute-api.us-east-1.amazonaws.com/prod/register
```

---

## 📊 **MONITOREO**

### CloudWatch Logs
- Cada Lambda crea su log group automáticamente
- `/aws/lambda/loginLambda`
- `/aws/lambda/getPostsLambda`
- etc.

### Métricas importantes
- Invocations
- Duration
- Errors
- Throttles

---

## 🎯 **CHECKLIST PARA EL EXAMEN**

- [ ] RDS MySQL configurado y accesible
- [ ] Base de datos twitter creada con tablas
- [ ] Layer pymysql creado y añadido
- [ ] Variables de entorno configuradas
- [ ] Security Groups configurados
- [ ] Lambdas funcionan en test
- [ ] API Gateway configurado
- [ ] CORS habilitado
- [ ] S3 Bucket creado y público
- [ ] Permisos IAM correctos

