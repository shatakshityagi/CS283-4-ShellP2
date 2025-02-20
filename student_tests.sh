#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Shell parses quoted strings correctly" {
    run bash -c "
        ./dsh <<EOF
echo \"hello,     world\" > quotes.txt
cat quotes.txt
EOF
    "
    
    [[ "$output" == *"hello,     world"* ]]
    rm -f quotes.txt
}

@test "Shell eliminates duplicate spaces" {
    run bash -c "
        ./dsh <<EOF
echo hello     world > spaces.txt
cat spaces.txt
EOF
    "
    
    [[ "$output" == *"hello world"* ]]
    rm -f spaces.txt
}

@test "External command ls works" {
    touch test_file.txt
    
    run bash -c "
        ./dsh <<EOF
ls test_file.txt
EOF
    "
    
    [[ "$output" == *"test_file.txt"* ]]
    rm -f test_file.txt
}

@test "External command with arguments works" {
    mkdir -p test_folder
    
    run bash -c "
        ./dsh <<EOF
ls -d test_folder
EOF
    "
    
    [[ "$output" == *"test_folder"* ]]
    rmdir test_folder
}

@test "Non-existent command returns error" {
    run bash -c "
        ./dsh <<EOF
nonexistentcommand
rc
EOF
    "
    
    [[ "$output" == *"Command not found"* ]]
    [[ "$output" == *"2"* ]]
}

@test "Permission denied error is handled" {
    run bash -c "
        touch temp_file
        chmod -x temp_file
        ./dsh <<EOF
./temp_file
rc
EOF
    "
    
    [[ "$output" == *"Permission denied"* ]]
    [[ "$output" == *"13"* ]]
    rm -f temp_file
}

@test "RC command shows successful command return code" {
    run bash -c "
        ./dsh <<EOF
echo hello
rc
EOF
    "
    
    [[ "$output" == *"0"* ]]
}

@test "RC command shows failed command return code" {
    run bash -c "
        ./dsh <<EOF
ls /nonexistentdirectory
rc
EOF
    "
    
    [[ "$output" == *"Command"*"2"* ]] || [[ "$output" == *"2"* ]]
}

@test "Output redirection works" {
    run bash -c "
        ./dsh <<EOF
echo testing redirection > redir_test.txt
cat redir_test.txt
EOF
    "
    
    [[ "$output" == *"testing redirection"* ]]
    rm -f redir_test.txt
}

@test "Multiple commands execute sequentially" {
    run bash -c "
        ./dsh <<EOF
echo first > test1.txt
echo second > test2.txt
cat test1.txt
cat test2.txt
EOF
    "
    
    [[ "$output" == *"first"* ]]
    [[ "$output" == *"second"* ]]
    rm -f test1.txt test2.txt
}

@test "Command with many arguments works" {
    run bash -c "
        ./dsh <<EOF
echo 1 2 3 4 5 6 7
EOF
    "
    
    [[ "$output" == *"1 2 3 4 5 6 7"* ]]
}

@test "Shell handles special characters" {
    run bash -c "
        ./dsh <<EOF
echo \$HOME > home_var.txt
cat home_var.txt
EOF
    "
    
    [[ "$output" == *"/"* ]]
    rm -f home_var.txt
}

@test "Return code is properly tracked between commands" {
    run bash -c "
        ./dsh <<EOF
ls /tmp
rc
ls /nonexistentpath
rc
echo success
rc
EOF
    "
    
    [[ "$output" == *"0"* ]]
    [[ "$output" == *"2"* ]] 
    [[ "$output" == *"success"* ]]
    [[ "$output" == *"0"* ]] 
}

