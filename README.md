# my base esp32 project

## 关于idf终端py环境的问题

### 如果esp-idf终端执行idf.py的时候提示Cannot import module "esp_idf_monitor". This usually means that "idf.py" was not spawned within an ESP-IDF shell environment or the python virtual environment used by "idf.py" is corrupted，在终端依次执行以下命令即可

``` shell
    $currentPath = $env:PATH
    $newPath ="D:\libs\esp32\tools\python_env\idf5.3_py3.11_env\Scripts"
    $env:PATH ="$newPath;$currentPath"
```

### tools目录

- move_files.py 主要将SquareLine Studio或者gui guider 生成的代码移动到符合当前项目结构的目录，并修改对应的include路径
