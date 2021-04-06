#!/bin/bash

# __readINI [配置文件路径+名称] [节点名] [键值]
function __readINI() {
    local INIFILE=$1;
    local SECTION=$2;
    local ITEM=$3;
    _readIni=`awk -F '=' '/\['$SECTION'\]/{a=1}a==1&&$1~/'$ITEM'/{print $2;exit}' $INIFILE`
    echo ${_readIni}
}

ctrl_d() {
    echo -e '\003'
}

build_resource() {
    cd ../src/Resource
    make clean
    make
    cd ../../test
}

build_sudoku() {
    cd ../src/Sudoku
    make clean
    make
    cd ../../test
}

create_file() {
    g++ multiplication.cpp -o multiplication.out
    rm *.test
    local l=${2}
    local c=${3}
    local f=$(__readINI config.ini testing file_name_keeper)
    rm ${f}
    for((i=1;i<=${1};i++));
    do
        ./multiplication.out test1000 ${2} ${c} test${l}.test
        echo "./test${l}.test" >> ${f}
        c=$[c+1]
        l=$[l*2]
    done
}

acc_test(){
    diff result1 result2 > acc.diff
    # cat acc.diff
}

thread_cmp(){
    local l=${2}
    local name=${3}
    for((i=1;i<=${1};i++))
    do
        declare starttime1=`date +%s%N`
        ../src/Resource/sudoku ./test${l}.test d >> result1
        declare endtime1=`date +%s%N`
        c=`expr $endtime1 - $starttime1`
        c=`expr $c / 1000000`
        echo "${l} ${c}" >> ${name}_single.log

        declare starttime2=`date +%s%N`
        echo "./test${l}.test" | ../src/Sudoku/sudoku -t8 >> result2
        declare endtime2=`date +%s%N`
        c=`expr $endtime2 - $starttime2`
        c=`expr $c / 1000000`
        echo "${l} ${c}" >> ${name}_multi.log
        l=$[l*2]
    done
}

test1(){
    local name=${1}
    local times=${2}
    local line=${3}
    for((i=1;i<=$times;i++));
    do
        declare starttime1=`date +%s%N`
        echo "./test${line}.test" | ../src/Sudoku/sudoku >> syy_test1.log
        declare endtime1=`date +%s%N`
        c=`expr $endtime1 - $starttime1`
        c=`expr $c / 1000000`
        echo "${line} ${c}" >> ${name}_to_file.log

        declare starttime2=`date +%s%N`
        echo "./test${line}.test" | ../src/Sudoku/sudoku 
        declare endtime2=`date +%s%N`
        c=`expr $endtime2 - $starttime2`
        c=`expr $c / 2000000`
        echo "${line} ${c}" >> ${name}_to_ternimal.log

        line=$[line*2]
    done
}

test2(){
    local name=${1}
    for((i=1;i<=30;i++));
    do
        declare starttime1=`date +%s%N`
        echo "./test128000.test" | ../src/Sudoku/sudoku -t${i} >> test1.log
        declare endtime1=`date +%s%N`
        c=`expr $endtime1 - $starttime1`
        c=`expr $c / 1000000`
        echo "${i} ${c}" >> ${name}_thread.log
    done
}

test3(){
    local name=${1}
    local block=100
    for((i=1;i<=20;i++));
    do
        declare starttime1=`date +%s%N`
        echo "./test128000.test" | ../src/Sudoku/sudoku -d${block} >> test1.log
        declare endtime1=`date +%s%N`
        c=`expr $endtime1 - $starttime1`
        c=`expr $c / 1000000`
        echo "${block} ${c}" >> ${name}_block.log
        block=$[block+200]
    done
}

# main

user_name=$(whoami| awk '{print $1}')
file_name="${user_name}.log"
uname -a > ${file_name}

file_num=($( __readINI config.ini testing file_num))
basic_line_num=($( __readINI config.ini testing basic_line_num))
file_names=$( __readINI config.ini testing file_name_keeper)
re_times=($( __readINI config.ini testing times))
TFN=($( __readINI config.ini testing thread_file_num))
TBLN=($( __readINI config.ini testing thread_basic_line_num))

if [ "1" ==  "$(__readINI config.ini func bulid_resource_f)" ]; then
    build_resource
fi

if [ "1" ==  "$(__readINI config.ini func bulid_sudoku_f)" ]; then
    build_sudoku
fi

if [ "1" ==  "$(__readINI config.ini func create_test_files_f)" ]; then
    create_file ${file_num} ${basic_line_num} 0
fi

for ((ii=1;ii<=${re_times};ii++));
do

    thread_cmp ${TFN} ${TBLN} ${user_name}
    test1 ${user_name} ${file_num} 1000
    test2 ${user_name}
    test3 ${user_name}
    if [ "1" ==  "$(__readINI config.ini func acc_test_f)" ]; then
        acc_test
    fi
done

python3 ./plots.py