#!/bin/bash

EXEC=albatrosscc
DUMMY=test
OUTPUT=output
TEST_DIR=tests
TIMEOUT=1s
DIFF_FILE=d
MARS=Mars4_5.jar
JAVA=java

PASSING=0

RED='\033[41;37m'
GREEN='\033[42m'
RESET='\033[0m'

export ASAN_OPTIONS="detect_leaks=false"
export MallocNanoZone="0"

make clean && make

START=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
for T in $(ls $TEST_DIR | grep -E "$1" | sort)
do
    PASS=1
    for F in $(ls $TEST_DIR/$T | grep ".albatross$" | grep pass | sort)
    do
        echo -n -e "Running ${GREEN}positive test${RESET} $T/$F"
        echo -n $'\t'
        TESTFILE="$TEST_DIR/$T/$F"
        EXPECTED=$(sed 's/.albatross/.expected/g' <<<"$TESTFILE")
        cp $TESTFILE $DUMMY
        mv $EXEC.exe $EXEC &> /dev/null
        T1=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        ./$EXEC $TESTFILE out.mips &> $DIFF_FILE
        RET=$?
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        if [ $RET -eq 0 ]
        then
            echo -e -n "${GREEN}COMPILE OK in ${TT}ms ${RESET}\t"
        else
            echo -e "${RED}COMPILE FAIL in ${TT}ms ${RESET}"
            cat $DIFF_FILE
            PASS=0
            continue
        fi

        T1=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        $JAVA -Xmx1G -jar $MARS nc out.mips > $OUTPUT
        RET=$?
        T2=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
        TT=$((T2-T1))

        if [ $RET -eq 0 ]
        then
            echo -e -n " ${GREEN}RUN OK in ${TT}ms${RESET}\t"
        else
            echo -e " ${RED}RUN FAIL in ${TT}ms${RESET}"
            cat $OUTPUT
            PASS=0
            continue
        fi


        # Remove possible Windows \r from output and expected files
        sed 's/\r$//' -i $OUTPUT   &> /dev/null
        sed 's/\r$//' -i $EXPECTED &> /dev/null
        diff $EXPECTED $OUTPUT &> $DIFF_FILE
        DIFF=$?
        rm $DUMMY $OUTPUT &> /dev/null

        if [ $DIFF -eq 0 ]
        then
            echo -e " ${GREEN}DIFF OK${RESET}\t"
        else
            echo -e " ${RED}DIFF FAIL${RESET}"
            cat $DIFF_FILE
        fi

        if [ $RET -ne 0 ] || [ $DIFF -ne 0 ]
        then
            PASS=0
            #continue 2
        fi
    done

    for F in $(ls $TEST_DIR/$T |  grep ".albatross$" |grep fail | sort)
    do
        echo -n -e "Running ${RED}negative test${RESET} $T/$F"
        echo -n $'\t'
        TESTFILE="$TEST_DIR/$T/$F"
        cp $TESTFILE $DUMMY
        $(./$EXEC $DUMMY &> /dev/null)
        RET=$?
        if [ $RET -eq 3 ]
        then
            echo -e "${GREEN}RET OK${RESET}"
        else
            echo -e "${RED}RET FAIL${RESET}"
            #continue 2
            PASS=0
        fi
        rm $DUMMY
    done

    if [ $PASS -eq 0 ]
    then
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
            echo -e "${RED}                   TEST ${T} FAILING                  ${RESET}"
    else
            echo -e "${GREEN}                   TEST ${T} PASSING                ${RESET}"
            echo -e "${GREEN}                   TEST ${T} PASSING                ${RESET}"
            echo -e "${GREEN}                   TEST ${T} PASSING                ${RESET}"
    fi

    PASSING=$(($PASSING+$PASS))
done
END=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
echo "Finished in $((END-START))ms"

echo $PASSING
