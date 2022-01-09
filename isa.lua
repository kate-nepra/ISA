isa_proto = Proto("ISAproto", "ISA Protocol") -- declare the protocol

-- create and assign the fields to ISAproto
message_length = ProtoField.int32("isa.message_length", "Message length", base.DEC)
state = ProtoField.string("isa.state", "State", base.ASCII)
i_data = ProtoField.string("isa.data", "Data", base.ASCII)
isa_proto.fields = {message_length, state, i_data}

-- declare dst and src port fields for reading
tcp_src_f = Field.new("tcp.srcport")
tcp_dst_f = Field.new("tcp.dstport")

-- dissecting function
function isa_proto.dissector(buffer, pinfo, tree)
    length = tostring(buffer:len()) -- get message length
    if length == 0 then
        return
    end

    if string.char(buffer(length - 1, 1):uint()) ~= ")" then -- if packet does not end properly try desegment one more segment
        pinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT
    end

    if tostring(buffer:reported_len()) ~= length then -- if packet is cut off due to user limit, do not dissect
        return 0
    end

    -- cut message status from message data char by char
    local t = {}
    local i = 0
    while string.char(buffer(i, 1):uint()) ~= " " do
        t[i] = tostring(string.char(buffer(i, 1):uint()))
        i = i + 1
    end

    state_str = table.concat(t, "") -- make string from the char array

    local tcp_src = tcp_src_f() -- get source port
    local tcp_dst = tcp_dst_f() -- get destination port

    -- decision on the direction on the basis of the port
    if tostring(tcp_src) == "32323" then
        if (state_str ~= "ok" and state_str ~= "err") then
            state_str = "Server response"
        else
            state_str = "Server response: " .. state_str
        end
    end

    if tostring(tcp_dst) == "32323" then
        state_str = "Client request: " .. state_str
    end

    pinfo.cols.protocol = isa_proto.name

    pinfo.cols.info = state_str
    local subtree = tree:add(isa_proto, buffer(), "ISA Protocol Data") -- create Protocol tree

    -- add items to the tree
    subtree:add(message_length, length)
    subtree:add(state, state_str)
    subtree:add(i_data, buffer(1, length - 2))
end

tcp_port = DissectorTable.get("tcp.port")
tcp_port:add(32323, isa_proto) -- register the protocol
