from pynodered import node_red
import re
import requests

@node_red(category="temperature")
def getapiclient(node, msg):
    url = "http://192.168.0.2/cgi-bin/param.cgi?userName=admin&password=admin&action=get&type=areaTemperature&AreaID=1"
    response = requests.get(url)
    msg['payload'] = str(response.content)
    return msg

@node_red(category="temperature")
def max_temp(node, msg):
    s = msg['payload']
    nindex = re.search("maxTemperature=", s).span()[1]
    st = str(s[nindex:nindex+5])
    msg['payload'] = float(st)
    return msg

@node_red(category="temperature")
def value_temp(node, msg):
    array = str(msg['payload']).split("\n")
    msg['payload'] = array[5]
    return msg