/**
    PebbleServ
    this file will be consumed by pebblejs
*/

var initialised = false;

// CONFIGURE FOR YOUR SERVER
var defaults = {
    ip: '104.131.117.58',
    port: '80',
    endpoint: '/PebbleServ/',
    jsonfile: 'pebbleServ.json',
    update_interval: 1000,
    auto_update: 1
}

function fetchServerStats(configs, FIELD) {
    var response;
    var URL = "http://" + configs.ip + ":" + configs.port + configs.endpoint + configs.jsonfile;
    
    var req = new XMLHttpRequest();
    req.TimeOut = 2000;
    req.open('GET', URL, true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {
                response = JSON.parse(req.responseText);
                var cpu, mem, host;
                if (response.Data) {
                    cpu = response.Data.cpu;
                    mem = response.Data.mem;
                    host = response.Data.host;
                    switch(FIELD) {
                        case 'all':
                            Pebble.sendAppMessage({
                                "mem": mem.toString(), "cpu": cpu.toString(), "host": host.toString()
                            });
                        break;
                        case 'host':
                            Pebble.sendAppMessage({
                                "host": host.toString()
                            });
                        break;
                        case 'cpu':
                            Pebble.sendAppMessage({
                                "cpu": cpu.toString()
                            });
                        break;
                        case 'mem':
                            Pebble.sendAppMessage({
                                "mem": mem.toString()
                            });
                            break;
                        default:
                            console.log('Invalid FIELD passed to fetchServerStats function');
                    }
                }
            } else {
                console.log("Request returned error code " + req.status.toString());
            }
        } else {
            console.log("Failed to get JSON object from server at " + URL);
        }
    }

    req.ontimeout = function() {
        console.log("Failed to get JSON object from server at " + URL);
    }
  
    req.ontimeout = function(f) {
        console.log('timeout');
    }
    
    req.onabort = function(f) {
        console.log('error');
    }
  
    req.send(null);
}
  
// Callback for app ready event
Pebble.addEventListener("ready", function(e) {
    ip = localStorage.getItem("ip");
    if (!ip)
        ip = defaults.ip;

    port = localStorage.getItem("port");
    if (!port)
        port = defaults.port;

    update_interval = localStorage.getItem("update_interval");
    if (update_interval == -1)
        update_interval = 1000;

    auto_update = localStorage.getItem("auto_update");
    if (auto_update == -1)
        auto_update = 1;

    initialised = true;
});

// Callback for phone app configuration window
Pebble.addEventListener("showConfiguration", function() {
    URL = "http://" + defaults.ip + ":" + defaults.port + defaults.endpoint + defaults.jsonfile;
    Pebble.openURL('http://' +  defaults.ip + defaults.endpoint + '/' +
        "ip=" + defaults.ip + 
        "&port=" + defaults.port +
        "&update_interval=" + (defaults.update_interval/1000).toString() +
        "&auto_update=" + defaults.auto_update.toString()
    );
});

// Callback for saving configuration options
Pebble.addEventListener("webviewclosed", function(e) {
    var options = JSON.parse(decodeURIComponent(e.response));
    
    ip = options.ip
    localStorage.setItem("ip", ip);

    port = options.port;
    localStorage.setItem("port", port);

    update_interval = parseInt(options.update_interval) * 1000;
    localStorage.setItem("update_interval", update_interval);

    auto_update = parseInt(options.auto_update);
    localStorage.setItem("auto_update", auto_update);

    watchConfig = {"ip": ip, "port": port, "auto": auto_update.toString(), "update_int": update_interval.toString()};

    Pebble.sendAppMessage(watchConfig);
});

// Callback for appmessage events
Pebble.addEventListener("appmessage", function(e) {
    configs = {
        ip: defaults.ip,
        port: defaults.port,
        endpoint: defaults.port,
        jsonfile: defaults.jsonfile
    }

    if (e.payload.fetch) {
        Pebble.sendAppMessage({
            "ip": defaults.ip,
            "port": defaults.port,
            "auto": defaults.auto_update.toString(),
            "update_int": defaults.update_interval.toString()
        });

        Pebble.sendAppMessage({"ip": defaults.ip});
        Pebble.sendAppMessage({"auto": defaults.auto_update.toString()});
        Pebble.sendAppMessage({"update_int": defaults.update_interval.toString()});

        fetchServerStats(configs, 'fetch');
    }
    
    if (e.payload.host)
        fetchServerStats(configs, 'host');
    
    if (e.payload.cpu)
        fetchServerStats(configs, 'cpu');
    
    if (e.payload.mem)
        fetchServerStats(configs, 'mem');

    if (e.payload.all)
        fetchServerStats(configs, 'all');
);


