const starterTable = [
  "buy followers",
  "want big follows",
  "want to buy"
];

function firstMessageLooksLikeSpam(message)
{
  var lcMessage = message.replace(/\s+/g, " ")
                         .toLowerCase();
  var result = false;

  for (i = 0;i < starterTable.length;i++) {
    var s = starterTable[i];

    if (lcMessage.startsWith(s)) {
      result = true;
      break;
    }
  }

  return result;
}

// Make sure it's not using fancy unicode to dodge word filters
function showMessageUnicodeInConsole(displayName, message)
{
  for (i = 0;i < message.length;i++) {
    var m = message.charCodeAt(i);

    if (m > 128) {
      console.log(`${displayName}'s message has code point ${m}!`);
    }
  }
}

module.exports = {
  firstMessageLooksLikeSpam,
  showMessageUnicodeInConsole
};
