#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output")
    output=$(ls)
    # Expected output with all whitespace removed for easier matching
    expected_output="$output
local mode
dsh4> dsh4> 
cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check echo works with leading, trailing, and inner spaces" {
    run ./dsh <<EOF                
         echo          hello      world               
EOF

    # Expected output needs white spaces to check for it
    expected_output="hello world
local mode
dsh4> dsh4> 
cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    # Check exact match
    [ "$output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check echo works with quotes" {
    run ./dsh <<EOF                
     echo          "hello      world"               
EOF
    # Expected output needs white spaces to check for it
    expected_output="hello      world
local mode
dsh4> dsh4> 
cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    # Check exact match
    [ "$output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check echo works with quotes within word" {
    run ./dsh <<EOF                
     echo   "hel"lo    world               
EOF
    # Expected output needs white spaces to check for it
    expected_output="hello world
local mode
dsh4> dsh4> 
cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    # Check exact match
    [ "$output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check echo works with multiple quotes" {
    run ./dsh <<EOF                
   echo  "  "amazin" g way " to" form""" a"t " 
EOF
    # Expected output needs white spaces to check for it
    expected_output="  amazin g way  to form at 
local mode
dsh4> dsh4> 
cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    # Check exact match
    [ "$output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check exit command terminates shell" {
    run ./dsh <<EOF                
exit            
EOF
    # Expected output needs white spaces to check for it
    expected_output="local mode
dsh4> cmd loop returned 0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    # Check exact match
    [ "$output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check for no commands" {
    run ./dsh <<EOF                

EOF
    # Expected output needs white spaces to check for it
    expected_output="local mode
dsh4> warning: no commands provided
dsh4> 
cmd loop returned -1"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    # Check exact match
    [ "$output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "check invalid command returns error" {
    run ./dsh <<EOF
foobar
EOF

    expected_output="local mode
dsh4> Command not found in PATH
local mode
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "comprehensive test: mkdir, cd, touch, uname, echo, ls, and pwd" {
    run ./dsh <<EOF
mkdir testdir
touch testdir/"file 1.txt"
cd testdir
ls
cd ..
rm -rf testdir
uname
echo "hello world"
pwd
EOF
    CWD=$(pwd)
    expected_output="file 1.txt
Linux
hello world
$CWD
local mode
dsh4> dsh4> dsh4> dsh4> dsh4> dsh4> dsh4> dsh4> dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "Expected Output:"
    echo "$expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "check rc command works on error" {
    run ./dsh <<EOF
not_exists
rc
EOF

    expected_output="local mode
dsh4> Command not found in PATH
local mode
dsh4> dsh4> 2
dsh4> 
cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "Expected Output:"
    echo "$expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "check rc command works on success" {
    run ./dsh <<EOF
echo
rc
EOF

    expected_output="
local mode
dsh4> dsh4> 0
dsh4> 
cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "Expected Output:"
    echo "$expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "only pipes" {
    run ./dsh <<EOF
|||
EOF

    expected_output="local mode
dsh4> warning: no commands provided
dsh4> 
cmd loop returned -1"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "Expected Output:"
    echo "$expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "piping echo into tr" {
    run ./dsh <<EOF
echo "hello world" | tr '[:lower:]' '[:upper:]'
EOF

    expected_output="HELLO WORLD
local mode
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "${output} -> ${expected_output}"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "more complicated pipe command using echo, tr, and sort" {
    run ./dsh <<EOF
echo "one two three" | tr ' ' '\n' | sort
EOF

    expected_output="one
three
two
local mode
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "poorly formatted pipe command: extra pipe" {
    run ./dsh <<EOF
echo "hello world" | | tr '[:lower:]' '[:upper:]'
EOF

    expected_output="local mode
dsh4> error: piping is improperly formatted
dsh4> 
cmd loop returned -4"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "piping with built-in command" {
    # Get the expected pwd (converted to uppercase) from the test environment

    run ./dsh <<EOF
cd | pwd
EOF

    CWD=$(pwd)
    expected_output="$CWD
local mode
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "multiple pipes together" {
    run ./dsh <<'EOF'
echo "Hello Pipe Test" | tr '[:upper:]' '[:lower:]' | awk '{print $2,$3}'
EOF

    expected_output="pipe test
local mode
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "too many pipes" {
    run ./dsh <<EOF
echo | echo | echo | echo | echo | echo | echo | echo | echo | echo | echo | echo | echo
EOF

    expected_output="local mode
dsh4> error: piping limited to 8 commands
dsh4> 
cmd loop returned -2"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "output redirection" {
    run ./dsh <<'EOF'
echo "hello, class" > out.txt
cat out.txt
EOF

    expected_output="hello, class
local mode
dsh4> dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    # Clean up the output file
    rm -f out.txt
}

@test "input redirection" {
    # Create a temporary file with content
    echo "input file content" > in.txt

    run ./dsh <<'EOF'
cat < in.txt
EOF

    expected_output="input file content
local mode
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    # Clean up the input file
    rm -f in.txt
}

@test "piped redirection, combining all redirections and pipes" {
    # Create a temporary input file
    echo "Hello Pipe Test" > in.txt

    run ./dsh <<'EOF'
cat < in.txt | tr '[:upper:]' '[:lower:]' > out.txt
cat out.txt
EOF

    expected_output="hello pipe test
local mode
dsh4> dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    # Clean up both temporary files
    rm -f in.txt out.txt
}

@test "redirection with append test" {
    run ./dsh <<'EOF'
echo "hello, class" > out.txt
cat out.txt
echo "this is line 2" >> out.txt
cat out.txt
EOF

    expected_output="hello, class
hello, class
this is line 2
local mode
dsh4> dsh4> dsh4> dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    # Clean up any temporary file created during the test
    rm -f out.txt
}

# These tests require a running server to connect to
# The server must be running on the default port 7982
@test "exiting the client" {
    run ./dsh -c <<'EOF'
exit
EOF

    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "exiting the client with a unique ip and port" {
    run ./dsh -c -i 0.0.0.0 -p 7982 <<'EOF'
exit
EOF

    expected_output="socket client mode:  addr:0.0.0.0:7982
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "dragon command on client" {
    run ./dsh -c <<'EOF'
dragon
exit
EOF

    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4>                                                                         @%%%%                       
                                                                     %%%%%%                         
                                                                    %%%%%%                          
                                                                 % %%%%%%%           @              
                                                                %%%%%%%%%%        %%%%%%%           
                                       %%%%%%%  %%%%@         %%%%%%%%%%%%@    %%%%%%  @%%%%        
                                  %%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%          
                                %%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%% %%%%%%%%%%%%%%%           
                               %%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%     %%%            
                             %%%%%%%%%%%%%%%%%%%%%%%%%%%%@ @%%%%%%%%%%%%%%%%%%        %%            
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%                
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              
                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%@              
      %%%%%%%%@           %%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%      %%                
    %%%%%%%%%%%%%         %%@%%%%%%%%%%%%           %%%%%%%%%%% %%%%%%%%%%%%      @%                
  %%%%%%%%%%   %%%        %%%%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%%%%%%%                        
 %%%%%%%%%       %         %%%%%%%%%%%%%             %%%%%%%%%%%%@%%%%%%%%%%%                       
%%%%%%%%%@                % %%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%                     
%%%%%%%%@                 %%@%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  
%%%%%%%@                   %%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              
%%%%%%%%%%                  %%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%  
%%%%%%%%%@                   @%%%%%%%%%%%%%%         %%%%%%%%%%%%@ %%%% %%%%%%%%%%%%%%%%%   %%%%%%%%
%%%%%%%%%%                  %%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%% %%%%%%%%%
%%%%%%%%%@%%@                %%%%%%%%%%%%%%%%@       %%%%%%%%%%%%%%     %%%%%%%%%%%%%%%%%%%%%%%%  %%
 %%%%%%%%%%                  % %%%%%%%%%%%%%%@        %%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%%%% %%
  %%%%%%%%%%%%  @           %%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%% 
   %%%%%%%%%%%%% %%  %  %@ %%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%% 
    %%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%           @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%% 
     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%   
      @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%               
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%  %%%%%%%          
           %%%%%%%%%%%%%%%%%%%%%%%%%%                           %%%%%%%%%%%%%%%  @%%%%%%%%%         
              %%%%%%%%%%%%%%%%%%%%           @%@%                  @%%%%%%%%%%%%%%%%%%   %%%        
                  %%%%%%%%%%%%%%%        %%%%%%%%%%                    %%%%%%%%%%%%%%%    %         
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%            
                %%%%%%%%%%%%%%%%%%%%%%%%%%  %%%% %%%                      %%%%%%%%%%  %%%@          
                     %%%%%%%%%%%%%%%%%%% %%%%%% %%                          %%%%%%%%%%%%%@          
                                                                                 %%%%%%%@       
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "cd and other commands work as expected" {
    run ./dsh -c <<'EOF'
pwd
cd ..
pwd
cd 6-ShellP4
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> $CWD
dsh4> dsh4> $parent_dir
dsh4> dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "piping commands from client to server" {
    run ./dsh -c <<'EOF'
echo "hello world" | tr '[:lower:]' '[:upper:]'
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> HELLO WORLD
dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}


@test "piping multiple commands from client to server" {
    run ./dsh -c <<'EOF'
echo "hello world" | tr '[:lower:]' '[:upper:]' | awk '{print $1}'
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> HELLO
dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "piping built-in commands from client to server" {
    run ./dsh -c <<'EOF'
dragon | wc -l
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> 38
dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "invalid command from client to server" {
    run ./dsh -c <<'EOF'
foobar
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> Command not found in PATH
dsh4> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

# remember to add exit command at the end of the test to fix bugs
@test "incorrect piping format from client to server" {
    run ./dsh -c <<'EOF'
echo "hello world" | | tr '[:lower:]' '[:upper:]' | cd ..
exit
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> error: piping is improperly formatted
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "too many pipes from client to server" {
    run ./dsh -c <<'EOF'
echo | echo | echo | echo | echo | echo | echo | echo | echo | echo | echo | echo | echo
exit
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> error: too many commands in pipeline
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "no commands from client to server" {
    run ./dsh -c <<'EOF'

exit
EOF

    CWD=$(pwd)
    parent_dir=$(dirname $CWD)
    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> warning: no commands provided
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "check rc command works on error from client to server" {
    run ./dsh -c <<'EOF'
not_exists
rc
exit
EOF

    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> Command not found in PATH
dsh4> 2
dsh4> cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "Expected Output:"
    echo "$expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "redirect input file from client to server" {
# temp file
echo "input file content" > in.txt
    run ./dsh -c <<'EOF'
cat < in.txt
exit
EOF

    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> input file content
dsh4> cmd loop returned 0"

    echo "Captured stdout:" 
    echo "$output"
    echo "Exit Status: $status"
    echo "Expected Output:"
    echo "$expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    # delete file
    rm -f in.txt
}

# stops the server from, running, must be second to last test
@test "stop-server command on client, immediate exit" {
    run ./dsh -c <<'EOF'
stop-server
EOF

    expected_output="socket client mode:  addr:127.0.0.1:7982
dsh4> cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

# This should be the last test in the file
# It checks if the server is no longer running
@test "should no longer be able to connect to server" {
    run ./dsh -c <<'EOF'
EOF

    expected_output="start client: Connection refused
socket client mode:  addr:127.0.0.1:7982
cmd loop returned -52"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}