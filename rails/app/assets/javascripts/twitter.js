var Twitter = (function() {
    // Array of tweets
    // Each tweet is an array [ id, text ]
    var tweets = [];
    var tweetsWereUpdated = false;
    
    var getTweets = function() {
        return tweets;
    };
    
    var haveNewTweets = function() {
        return tweetsWereUpdated;
    };
    
    var updateTweets = function() {
        var dfd = new $.Deferred();
	    var req = $.ajax({
		    url : Config.tweetsPath,
		    type : 'get'
	    }).done(function(data, status, xhr) {
	        var same = false;
	        var i;
	        if (tweets.length == data.length) {
	            same = true;
	            for (i = 0; i < tweets.length; i++) {
	                if (tweets[i][0] !== data[i][0]) {
	                    same = false;
	                    break;
	                }
	            }
	        }
	        tweetsWereUpdated = !same;
		    tweets = data;
		    dfd.resolve(tweetsWereUpdated);
	    }).fail(function(xhr, status, e) {
		    var errMsg = e;
		    if (xhr.status === 500) {
			    errMsg = xhr.responseText;
		    }
		    FlashMessage.error('Fehler beim Laden des Tweets: ' + status + errMsg);
		    dfd.reject(e);
	    });
	    return dfd.promise();
    };
    
    return {
        getTweets: getTweets,
        updateTweets: updateTweets,
        haveNewTweets: haveNewTweets
    };
}());
