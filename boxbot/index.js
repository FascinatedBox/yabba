const config = require('./.config.json');
const TMI = require('tmi.js');
const WebSocket = require('ws');
const url = 'ws://localhost:8002';
const bog = require('./bog.js');
var queue = [

];
var queue_open = true;

var actually_connect_to_tmi = true;

var websock = null;
var tmiClient = null;
var id = null;
var level_regex = /^[\w\d]{4}[\- ][\w\d]{4}\s/;

function wsSend(blob)
{
  websock.send(JSON.stringify(blob));
}

function command_list(channel, tags, message)
{
  var username = tags['display-name'];

  if (queue.length == 0) {
    tmiClient.say(channel, `@${username}: Queue is currently empty.`);
    return;
  }

  var names = queue.map(function(f) { return f['user']; })
           .join(", ");

  tmiClient.say(channel, `@${username}: Currently in queue: ${names}.`);
  console.log(queue);
}

function is_broadcaster(tags)
{
  if (tags['badges'] == null)
    return false;
  return (tags['badges']['broadcaster'] === "1");
}

function add_level_to_queue(channel, username, level)
{
  var found = false;

  for (var i = 0;i < queue.length;i++) {
    var u = queue[i]['user'];

    if (u == username) {
      found = true;
      break;
    }
  }

  queue_entry = {'user': username, 'level': level};

  if (found == false) {
    i++;
    queue.push(queue_entry);
    tmiClient.say(channel, `@${username}: Added ${level} to queue at position #${i}.`);
    wsSend({
      'op': 'addLevel',
      'level': level,
      'index': i,
      'user': username
    });
  }
  else {
    queue[i] = queue_entry;
    tmiClient.say(channel, `@${username}: Updated #${i + 1} to "${level}" (only one level at a time).`);
    wsSend({
      'op': 'replaceLevel',
      'level': level,
      'index': i,
      'user': username
    });
  }
}

function command_addlevel(channel, tags, message)
{
  var levelcode = message.split(/ +/)[1];
  var username = tags['display-name'];

  if (levelcode === undefined)
    return;

  levelcode = levelcode.slice(0, 9) + " ";

  if (levelcode.match(level_regex) === null) {
    tmiClient.say(channel, `@${username}: Invalid level code or the regex I'm using is wrong.`);
    return;
  }

  levelcode = levelcode.slice(0, 9)
                       .replace(" ", "-")
                       .toUpperCase();

  if (levelcode === "QQKX-AJBL") {
    tmiClient.say(channel, `@${username}: No.`);
    return;
  }

  if (levelcode === "QGVB-HCA6") {
    tmiClient.say(channel, `@${username}: HOSTILE LEVELCODE DETECTED.`);
    return;
  }

  if (queue_open == false &
      is_broadcaster(tags) === false) {
    tmiClient.say(channel, `@${username}: Box is tired (queue is closed)`);
    return;
  }

  add_level_to_queue(channel, username, levelcode);
}

function command_addlevelas(channel, tags, message)
{
  if (is_broadcaster(tags) === false) {
    tmiClient.say(channel, "lolno");
    return;
  }

  var args = message.split(/ +/);
  var username = args[1];
  var levelcode = args[2];

  add_level_to_queue(channel, username, levelcode);
}

function command_leave(channel, tags, message)
{
  var username = tags['display-name'];
  var found = false;

  for (var i = 0;i < queue.length;i++) {
    var u = queue[i]['user'];

    if (u == username) {
      found = true;
      break;
    }
  }

  if (found === false)
    tmiClient.say(channel, `@${username}: You are not in the queue.`);
  else {
    wsSend({
      'op': 'removeLevel',
      'index': i,
    });

    queue.splice(i, 1);

    tmiClient.say(channel, `@${username}: Removed from queue.`);
  }
}

function command_next(channel, tags, message)
{
  if (is_broadcaster(tags) === false)
    return;

  var caster = tags['display-name'];

  if (queue.length === 0) {
    tmiClient.say(channel, `@${caster}: No levels remaining.`);
    return;
  }

  var pair = queue.shift();
  var username = pair['user'];
  var level = pair['level'];
  var count = queue.length;

  tmiClient.say(channel, `@${caster}: Next person is @${username} with "${level}" (${count} remaining).`);

  wsSend({
    'op': 'pickLevel',
    'level': level,
    'index': 0,
    'user': username
  });

  // This code is 9 chars long and matches the regexp.
  // may want extra bulletproofing though.
//  const ls = exec('./next-level.sh ' + level, function (error, stdout, stderr) {
//    if (error) {
//    console.log(error.stack);
//    console.log('Error code: ' + error.code);
//    console.log('Signal received: ' + error.signal);
//    }
//    console.log('Child Process STDOUT: ' + stdout);
//    console.log('Child Process STDERR: ' + stderr);
//  });
}

function command_closeq(channel, tags, message)
{
  if (is_broadcaster(tags) === false)
    return;

  var username = tags['display-name'];
  var size = queue.length;

  queue_open = false;
  tmiClient.say(channel, `@${username}: Queue is now closed (${size} left).`);
  wsSend({
    'op': 'closeQueue'
  });
}

function command_openq(channel, tags, message)
{
  if (is_broadcaster(tags) === false)
    return;

  var username = tags['display-name'];

  queue_open = true;
  tmiClient.say(channel, `@${username}: Queue is now open.`);
  wsSend({
    'op': 'openQueue'
  });
}

function command_commands(channel, tags, message)
{
  var displayName = tags['display-name'];
  var table = getCommandTable(tags);
  var command_str = Object.keys(table)
                          .sort()
                          .join(', ');

  tmiClient.say(channel, `@${displayName}: ${command_str}.`);
}

var userCommandTable = {
  "!addlevel":   command_addlevel,
  "!commands":   command_commands,
  "!leave":      command_leave,
  "!list":       command_list,
  "!queue":      command_list
};

var casterCommandTable = {
  "!addlevelas": command_addlevelas,
  "!closeq":     command_closeq,
  "!openq":      command_openq,
};

function getCommandTable(tags)
{
  if (is_broadcaster(tags))
    return casterCommandTable;
  else
    return userCommandTable;
}

function unhandled(data) {
  console.log(`Unhandled request: ${JSON.stringify(data)}`);
}

function connectTMI()
{
  tmiClient = new TMI.client(config);

  if (websock === null)
    console.log("WARNING: Not connected to yabba (are you sure?).");

  tmiClient.on('connected', function (addr, port) {
    console.log(`boxbox: Twitch up and running on ${addr}:${port}`);

    // Call it in a timeout to avoid an error being logged
    setTimeout(function () {
      console.log("Connected to chat.");
    }, 200);
  });

  tmiClient.on('message', function (channel, tags, message, self) {
    if (self) { return; }

    if (tags['first-msg'] === true) {
      var displayName = tags['display-name'];

      if (bog.firstMessageLooksLikeSpam(message)) {
        console.log(`boxbox: Spamming ${displayName} for '''${message}'''.`)
        tmiClient.ban(channel, displayName,
            "Your first message looks like spam.");
      }

      bog.showMessageUnicodeInConsole(displayName, message);
    }

    message = message.trim();

    let command = message.split(" ")[0];
    let table = getCommandTable(tags);
    let c = table[command];

    if (c)
      c(channel, tags, message);
  });

  if (actually_connect_to_tmi == false)
    console.log('boxbox: NOTE! Skipping tmi connection.');
  else
    tmiClient.connect();
}

function sock_init(data)
{
  id = data['id'];

  wsSend({
    'op': 'syncRequest',
    'id': id
  });
}

function sock_syncReply(data)
{
  if (data['id'] != id)
    return;

  console.log(data);

  queue = data['queue'];
  queue_open = data['queueOpen'];
}

var dispatchTable = {
  'init': sock_init,
  'clearLevel': function() {},
  'removeLevel': function() {},
  'queueClose': function() {},
  'queueOpen': function() {},
  'startLevel': function () {
    queue.shift();
  },
  'syncRequest': function () {},
  'stopLevel': function() {},
  'syncReply': sock_syncReply,
};

function yabbaConnect()
{
  console.log("boxbox: Connecting to yabba...");
  websock = new WebSocket(url);
 
  websock.onopen = () => {
    console.log('boxbox: Yabba connected, now connecting to twitch.');

    if (tmiClient === null)
      connectTMI();
  }
  websock.onmessage = (e) => {
    var data = JSON.parse(e.data);
    var fn = dispatchTable[data['op']] || unhandled;

    fn(data);
  }
  websock.onerror = (error) => {
    console.log(`boxbox: Yabba connect failed (reason: ${error.error}). Stopping.`);
  }
  websock.onclose = () => {
    console.log("boxbox: Yabba connection lost, will begin retrying.");
    setTimeout(yabbaConnect, 1000);
  }
}

function copyUserTableIntoCasterTable()
{
  var userKeys = Object.keys(userCommandTable);

  for (i = 0;i < userKeys.length;i++) {
    var k = userKeys[i];

    casterCommandTable[k] = userCommandTable[k];
  }
}

copyUserTableIntoCasterTable();
yabbaConnect();
