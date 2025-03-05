#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="batsdragon.cdshdsh_cli.cdshlib.cdshlib.hmakefilequestions.mddsh3>dsh3>cmdloopreturned0"

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
dsh3> dsh3> 
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
dsh3> dsh3> 
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
dsh3> dsh3> 
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
dsh3> dsh3> 
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
    expected_output="dsh3> cmd loop returned 0"

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
    expected_output="dsh3> warning: no commands provided
dsh3> 
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

    expected_output="dsh3> Command not found in PATH
dsh3> dsh3> 
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

    expected_output="file 1.txt
Linux
hello world
/home/dct55/cs283/5-ShellP3
dsh3> dsh3> dsh3> dsh3> dsh3> dsh3> dsh3> dsh3> dsh3> dsh3> 
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

    expected_output="dsh3> Command not found in PATH
dsh3> dsh3> 2
dsh3> 
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
dsh3> dsh3> 0
dsh3> 
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

    expected_output="dsh3> warning: no commands provided
dsh3> 
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
dsh3> dsh3> 
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
dsh3> dsh3> 
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

    expected_output="dsh3> error: piping is improperly formatted
dsh3> 
cmd loop returned -1"

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

    expected_output="/home/dct55/cs283/5-ShellP3
dsh3> dsh3> 
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
dsh3> dsh3> 
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

    expected_output="dsh3> error: piping limited to 8 commands
dsh3> 
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
dsh3> dsh3> dsh3> 
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
dsh3> dsh3> 
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
dsh3> dsh3> dsh3> 
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
dsh3> dsh3> dsh3> dsh3> dsh3> 
cmd loop returned 0"

    echo "Captured stdout: $output"
    echo "Exit Status: $status"
    echo "Expected Output: $expected_output"

    [ "$output" = "$expected_output" ]
    [ "$status" -eq 0 ]

    # Clean up any temporary file created during the test
    rm -f out.txt
}