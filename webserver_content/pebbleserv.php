<?php
    /**
     * PebbleServ
     * This file should be hosted on your server and available to the outside world
     * It is the file the app will try to hit when the user wants to change app configs
     */

    $ipaddr = !empty($_GET['ip']) ? htmlentities($_GET['ip']) : "192.168.0.1";
    $port = !empty($_GET['port']) ? htmlentities($_GET['port']) : "8080";
    
    $auto_update = !empty($_GET['auto_update']) ? htmlentities($_GET['auto_update']) : '1';
    $option1 = $auto_update=='1' ? "" : " selected";
    $option2 = $auto_update=='1' ? " selected" : "";

    $update_interval = !empty($_GET['update_interval']) ? htmlentities($_GET['update_interval']) : '1';
?>
<!DOCTYPE html>
<html>
<head>
    <title>PebbleServ Configs</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.css" />
</head>
<body>
    <div data-role="page" id="main">
        <div data-role="header" class="jqm-header">
            <h1>Server Monitor Configuration</h1>
        </div>

        <div data-role="content">
            <div data-role="fieldcontain">
                <label for="ip">Server IP Address:</label>
                <textarea cols="40" rows="8" name="ip" id="ip"><?=$ipaddr?></textarea>
            </div>

            <div data-role="fieldcontain">
                <label for="port">Server Port:</label>
                <textarea cols="40" rows="8" name="port" id="port"><?=$port?></textarea>
            </div>

            <div data-role="fieldcontain">
                <label for="auto_update">Auto-Refresh:</label>
                <select name="auto_update" id="auto_update" data-role="slider">
                    <option value="0" <?=$option1?>>Off</option>';
                    <option value="1" <?=$option2?>>On</option>
                </select>
            </div>

            <div data-role="fieldcontain">
                <label for="ip">Update interval [seconds]:</label>
                <textarea cols="40" rows="8" name="update_interval" id="update_interval"><?=$update_interval?></textarea>
            </div>

            <div class="ui-body ui-body-b">
                <fieldset class="ui-grid-a">
                    <div class="ui-block-a"><button type="submit" data-theme="d" id="b-cancel">Cancel</button></div>
                    <div class="ui-block-b"><button type="submit" data-theme="a" id="b-submit">Submit</button></div>
                </fieldset>
            </div>
        </div>
    </div>

    <script src="http://code.jquery.com/jquery-1.9.1.min.js"></script>
    <script src="http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.js"></script>
    <script>

        $(function() {

            // Event: close pebble config view
            $("#b-cancel").click(function() {
                document.location = "pebblejs://close";
            });

            // Event: submit pebble config view
            $("#b-submit").click(function() {
                document.location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(saveOptions()));
            });

        });

        // Returns the values inside dom fields
        function saveOptions() {
            return {
                'ip': $("#ip").val(),
                'port': $("#port").val(),
                'auto_update': $("#auto_update").val(),
                'update_interval': $("#update_interval").val(),
            }
        }
    </script>
</body>
</html>
