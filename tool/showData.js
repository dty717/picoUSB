var s = strToNumList = function (numStr) {
    var num = parseInt("0x" + numStr)
    console.log(num.toString(2))
    var numList = []
    var index = 0;
    while (num) {
        if (num & 1) {
            numList.push(index)
        }
        index++
        num = parseInt(num / 2)
        if (index > 100) {
            break
        }
    }
    return numList
}


    // 0x80000000 [31]    : DATA_SEQ_ERROR (0): Data Sequence Error
    // 0x40000000 [30]    : ACK_REC (0): ACK received
    // 0x20000000 [29]    : STALL_REC (0): Host: STALL received
    // 0x10000000 [28]    : NAK_REC (0): Host: NAK received
    // 0x08000000 [27]    : RX_TIMEOUT (0): RX timeout is raised by both the host and device if an ACK is not received in...
    // 0x04000000 [26]    : RX_OVERFLOW (0): RX overflow is raised by the Serial RX engine if the incoming data is too fast
    // 0x02000000 [25]    : BIT_STUFF_ERROR (0): Bit Stuff Error
    // 0x01000000 [24]    : CRC_ERROR (0): CRC Error
    // 0x00080000 [19]    : BUS_RESET (0): Device: bus reset received
    // 0x00040000 [18]    : TRANS_COMPLETE (0): Transaction complete
    // 0x00020000 [17]    : SETUP_REC (0): Device: Setup packet received
    // 0x00010000 [16]    : CONNECTED (0): Device: connected
    // 0x00000800 [11]    : RESUME (0): Host: Device has initiated a remote resume
    // 0x00000400 [10]    : VBUS_OVER_CURR (0): VBUS over current detected
    // 0x00000300 [9:8]   : SPEED (0): Host: device speed
    // 0x00000010 [4]     : SUSPENDED (0): Bus in suspended state
    // 0x0000000c [3:2]   : LINE_STATE (0): USB bus line state
    // 0x00000001 [0]     : VBUS_DETECTED (0): Device: VBUS Detected


function getSie_Status(numList){
    var dataStringList = []
    for (let index = 0; index < numList.length; index++) {
        const numIndex = numList[index];
        let bus_line_state = 0;
        let speed = 0;
        // dataStringList.push(numIndex);
        switch (numIndex) {
            case 0:
                dataStringList.push("VBUS Detected")
                break;
            case 2:
                bus_line_state += 1;
                break;
            case 3:
                bus_line_state += 2;
                break;
            case 4:
                dataStringList.push("Bus in suspended state")
                break;
            case 8:
                speed += 1;
                break;
            case 9:
                speed += 2;
                break;
            case 10:
                dataStringList.push("VBUS over current detected")
                break;
            case 11:
                dataStringList.push("Host: Device has initiated a remote resume. Device: host has initiated a resume.");
                break;
            case 16:
                dataStringList.push("Device: connected")
                break;
            case 17:
                dataStringList.push("Device: Setup packet received")
                break;
            case 18:
                dataStringList.push("Transaction complete.")
                break;
            case 19:
                dataStringList.push("Device: bus reset received")
                break;
            case 24:
                dataStringList.push("CRC Error. Raised by the Serial RX engine.")
                break;
            case 25:
                dataStringList.push("Bit Stuff Error. Raised by the Serial RX engine.")
                break;
            case 26:
                dataStringList.push("RX overflow is raised by the Serial RX engine if the incoming data is too fast.")
                break;
            case 27:
                dataStringList.push("RX timeout is raised by both the host and device if an ACK is not received in the maximum time specified by the USB spec.")
                break;
            case 28:
                dataStringList.push("Host: NAK received.")
                break;
            case 29:
                dataStringList.push("Host: STALL received")
                break;
            case 30:
                dataStringList.push("ACK received. Raised by both host and device.")
                break;
            case 31:
                dataStringList.push("Data Sequence Error.")
                break;
            default:
                dataStringList.push("unknown:" + numIndex)
                break;
        }
        if(bus_line_state){
            dataStringList.push("USB bus line state:" + bus_line_state)
        }
        if (speed) {
            switch (speed) {
                case 1:
                    dataStringList.push("Host: device low speed.")
                    break;
                case 2:
                    dataStringList.push("Host: device full speed.")
                    break;
                default:
                    dataStringList.push("Host: device unknown speed.")
                    break;
            }
        }

    }
    return dataStringList;
}

let Sie_Status = "Sie_Status"
// getSie_Status
function analyzeData(numList,dataName){
    switch (dataName) {
        case Sie_Status:
            return getSie_Status(numList);
            break;
    
        default:
            break;
    }
}

var ss = hexToBuffer = function (dataStr) {
    return Buffer.from(dataStr.trim().split(/[\s\t\r\n]+/).map(e => parseInt("0x" + e))).toString()
}
