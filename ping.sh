#!/bin/bash

ip_array=(localhost www.city.osaka.lg.jp www.metro.tokyo.lg.jp www.pref.hokkaido.lg.jp www.pref.okinawa.jp www.state.gov www.gov.uk www.india.gov.in)

for ip in "${ip_array[@]}"
do
  fping $ip -c 10
done
