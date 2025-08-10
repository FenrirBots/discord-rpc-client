echo off
setlocal
@REM Check if git exists on the path before continuing.
where git.exe >NUL 2>&1
if [%errorlevel%] neq [0] (
  echo Git could not be found on the system path.
  goto :end
)

@REM Just a random file to make sure we initialized our submodule's submodule.
if not exist vendor\discord-rpc\vendor\rapidjson\readme.md (
  git submodule update --init --recursive
)
:end
endlocal
:eof