# my base esp32 project

## 关于idf终端py环境的问题

### 如果esp-idf终端执行idf.py的时候提示Cannot import module "esp_idf_monitor". This usually means that "idf.py" was not spawned within an ESP-IDF shell environment or the python virtual environment used by "idf.py" is corrupted，在终端依次执行以下命令即可

``` shell
    $currentPath = $env:PATH
    $newPath ="D:\libs\esp32\tools\python_env\idf5.3_py3.11_env\Scripts"
    $env:PATH ="$newPath;$currentPath"
```
