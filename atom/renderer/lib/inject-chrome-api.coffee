bindings = process.atomBinding 'chrome_api_request_sender'
chrome_api_request_sender = bindings.chromeApiRequestSender

chrome = window.chrome = window.chrome || {}

v8Util = process.atomBinding 'v8_util'
v8Util.setHiddenValue global, 'chrome_api_request_sender', chrome_api_request_sender

requests =  {}

chrome_api_request_sender._handleResponse = (request_id, error_msg, result) ->
  if error_msg
    chrome.lastError = error_msg
    requests[request_id].callback()
  else
    delete chrome.lastError if chrome.lastError
    requests[request_id].callback result

  delete requests[request_id]

prepareRequest = (args) ->
  request = {}
  argCount = args.length

  if argCount > 0 and typeof args[argCount-1] is 'function'
    request.callback = args[--argCount]
  request.args = []
  for k in [0..argCount]
    request.args[k] = args[k]
  request

#var attachedEventNames = []
#chrome_api_request_sender._dispatchEvent = () ->


#function EventImpl (eventName) {
  #this.eventName = eventName
  #this.listeners = [];
#}

#EventImpl.addListener = function(listener) {
  #this.listeners_.push(listener);
  #if (this.listeners_.size == 1):
    #chrome_api_request_sender._attachEvent(this.eventName)
#}

#EventImpl.removeListener = function(listener) {
#}

chrome.test = ->
  chrome_api_request_sender._attachEvent "system.storage.onAttached"
#system.storage.onAttached

chrome.system =
  cpu:
    getInfo: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'system.cpu.getInfo', [], request.id
  memory:
    getInfo: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'system.memory.getInfo', [], request.id
  storage:
    getInfo: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'system.storage.getInfo', [], request.id
  network:
    getNetworkInterfaces: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'system.network.getNetworkInterfaces', [], request.id

chrome.serial =
  getDevices: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'serial.getDevices', [{}], request.id

chrome.hid =
  getDevices: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'hid.getDevices', [{}], request.id

chrome.usb =
  getUserSelectedDevices: ->
      request = prepareRequest arguments
      request.id = chrome_api_request_sender._getNextRequestID()
      requests[request.id] = request
      chrome_api_request_sender._sentRequest 'usb.getUserSelectedDevices', [{}], request.id
