@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo   Mosquitto TLS 证书生成脚本（无 IP/DNS 绑定）
echo ============================================
echo.

:: ============================================
:: 可按需修改的变量
:: ============================================

:: 证书有效期（天）
set DAYS=3650

:: 输出目录
set OUT_DIR=.

:: CA 信息
set CA_SUBJ=/C=CN/ST=Shanghai/L=Shanghai/O=MyOrg/OU=IoT/CN=MyCA

:: 服务端证书信息（CN 只是标识用，不参与主机名验证）
set SERVER_SUBJ=/C=CN/ST=Shanghai/L=Shanghai/O=MyOrg/OU=IoT/CN=mqtt-server

:: 客户端证书信息
set CLIENT_SUBJ=/C=CN/ST=Shanghai/L=Shanghai/O=MyOrg/OU=IoT/CN=mqtt-client

:: ============================================
:: 检查 openssl 是否可用
:: ============================================
where openssl >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未找到 openssl，请安装并加入 PATH。
    echo 下载: https://slproweb.com/products/Win32OpenSSL.html
    pause
    exit /b 1
)

:: ============================================
:: 创建输出目录
:: ============================================
if not exist %OUT_DIR% mkdir %OUT_DIR%
echo [1/6] 输出目录: %OUT_DIR%

:: ============================================
:: 1. 生成 CA 私钥 + 自签名证书
:: ============================================
echo [2/6] 生成 CA 私钥和自签名证书...
openssl genrsa -out %OUT_DIR%\ca.key 4096
if %errorlevel% neq 0 goto :error

openssl req -new -x509 -days %DAYS% ^
    -key %OUT_DIR%\ca.key ^
    -out %OUT_DIR%\ca.crt ^
    -subj "%CA_SUBJ%"
if %errorlevel% neq 0 goto :error
echo     OK: ca.key / ca.crt

:: ============================================
:: 2. 生成服务端私钥 + CSR + 签发证书
::    不加任何 SAN/extfile，证书不绑定 IP 或域名
:: ============================================
echo [3/6] 生成服务端证书...
openssl genrsa -out %OUT_DIR%\server.key 2048
if %errorlevel% neq 0 goto :error

openssl req -new ^
    -key %OUT_DIR%\server.key ^
    -out %OUT_DIR%\server.csr ^
    -subj "%SERVER_SUBJ%"
if %errorlevel% neq 0 goto :error

openssl x509 -req -days %DAYS% ^
    -in %OUT_DIR%\server.csr ^
    -CA %OUT_DIR%\ca.crt ^
    -CAkey %OUT_DIR%\ca.key ^
    -CAcreateserial ^
    -out %OUT_DIR%\server.crt
if %errorlevel% neq 0 goto :error
echo     OK: server.key / server.crt

:: ============================================
:: 3. 生成客户端私钥 + CSR + 签发证书
:: ============================================
echo [4/6] 生成客户端证书...
openssl genrsa -out %OUT_DIR%\client.key 2048
if %errorlevel% neq 0 goto :error

openssl req -new ^
    -key %OUT_DIR%\client.key ^
    -out %OUT_DIR%\client.csr ^
    -subj "%CLIENT_SUBJ%"
if %errorlevel% neq 0 goto :error

openssl x509 -req -days %DAYS% ^
    -in %OUT_DIR%\client.csr ^
    -CA %OUT_DIR%\ca.crt ^
    -CAkey %OUT_DIR%\ca.key ^
    -CAcreateserial ^
    -out %OUT_DIR%\client.crt
if %errorlevel% neq 0 goto :error
echo     OK: client.key / client.crt

:: ============================================
:: 4. 验证证书链
:: ============================================
echo [5/6] 验证证书链...
openssl verify -CAfile %OUT_DIR%\ca.crt %OUT_DIR%\server.crt
openssl verify -CAfile %OUT_DIR%\ca.crt %OUT_DIR%\client.crt

:: ============================================
:: 5. 清理临时文件
:: ============================================
echo [6/6] 清理临时文件...
del %OUT_DIR%\*.csr >nul 2>&1
del %OUT_DIR%\*.srl >nul 2>&1

:: ============================================
:: 输出摘要
:: ============================================
echo.
echo ============================================
echo   生成完成！文件清单：
echo ============================================
echo.
echo   %OUT_DIR%\ca.crt       <- 所有端都需要，用于验证对方证书
echo   %OUT_DIR%\ca.key       <- CA 私钥，妥善保管勿外泄
echo   %OUT_DIR%\server.crt   <- mosquitto broker 使用
echo   %OUT_DIR%\server.key   <- mosquitto broker 使用
echo   %OUT_DIR%\client.crt   <- 客户端使用（可复制到所有设备）
echo   %OUT_DIR%\client.key   <- 客户端使用（可复制到所有设备）
echo.
echo ============================================
echo   mosquitto.conf 配置：
echo ============================================
echo.
echo   listener 8883 0.0.0.0
echo   cafile   %CD%\%OUT_DIR%\ca.crt
echo   certfile %CD%\%OUT_DIR%\server.crt
echo   keyfile  %CD%\%OUT_DIR%\server.key
echo   require_certificate false
echo.
echo ============================================
echo   客户端连接命令（必须加 --insecure）：
echo ============================================
echo.
echo   mosquitto_pub -h ^<broker_ip^> -p 8883   ^
echo     --cafile %OUT_DIR%\ca.crt              ^
echo     --cert   %OUT_DIR%\client.crt          ^
echo     --key    %OUT_DIR%\client.key          ^
echo     --insecure                             ^
echo     -t test -m "hello"
echo.
echo   说明：--insecure 仅跳过 IP/域名匹配检查，
echo   证书签名验证仍然有效，内网环境下安全可用。
echo.
pause
exit /b 0

:error
echo.
echo [错误] 证书生成失败，请检查上方 OpenSSL 输出。
pause
exit /b 1
