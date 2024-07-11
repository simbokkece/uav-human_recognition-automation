from pynodered import node_red

@node_red(category="temperature")
def max_temp(node, msg):
    array = str(msg['payload']).split("\n")
    msg['payload'] = array[5]
    return msg