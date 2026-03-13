# TESTING STRATEGY

## TESTING QUEUE DRIVER

### 1. TESTING QUEUE INIT
Cases:
1. **Supplied a NULL pointer**
- Expected: QUEUE_ERR_INVALID_ARG;
- Receieved: QUEUE_ERR_INVALID_ARG 
- Status: ✅ PASS
2. **Supplied 0 capacity**
- Expected: QUEUE_ERR_INVALID_ARG;
- Received: QUEUE_ERR_INVALID_ARG
- Status: ✅ PASS
3. **Can't allocate memory for queue instance**
- Expected: QUEUE_ERR_NO_MEMORY
- Received: Don't know how to simulate
- Status: ⚠️ NOT TESTED
4. **Can't allocate memory for buffer (Supplied negative capacity)**
- Expected: QUEUE_ERR_NO_MEMORY & Queue instance to be cleared
- Received: QUEUE_ERR_NO_MEMORY
- Status: ✅ PASS
5. **Supplied with valid pointer & valid capacity**
- Expected: Initialisation pass message, instance contents and buffer contents
- Received: Sucess messages
- Status: ✅ PASS

### TESTING QUEUE SEND
1. **Supplied with NULL queue argument** 
- Expected: QUEUE_ERR_INVALID_ARG;
- Receieved: QUEUE_ERR_INVALID_ARG 
- Status: ✅ PASS
2. **Supplied with NULL data & !0 size arguments** 
- Expected: QUEUE_ERR_INVALID_ARG;
- Receieved: QUEUE_ERR_INVALID_ARG 
- Status: ✅ PASS
3. **Supplied with NULL data & 0 size arguments** 
- Expected: No data printed to buffer
- Receieved: No data printed to buffer ⚠️ (head pointer moved 2 bytes)
- Status: ⚠️ FAIL
- Updated: Don't allow 0 size arugment ✅
- Status: ✅ PASS
4. **Supplied with data & (size < datasize) arguemnts**
- Expected: Only n(size) bytes of data passed through
- Receieved: n(size) bytes of data
- Status: ✅ PASS
5. **Payload size > uint16_t for lengths(65535)**
- Expected: System error
- Receieved:  
- Status: ⚠️ NOT TESTED

