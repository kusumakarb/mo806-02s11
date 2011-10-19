#!/bin/bash

/usr/bin/time -f "User:%U\nSys:%S\nTotal:%E\nContexSwitch:%w" ./yes_aux.sh $1
