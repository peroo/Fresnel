var util = (function()
{
    var pad = function(num) {
        return num < 10 ? '0' + num : num;
    }

    var formatTime = function(sec) {
        return Math.floor(sec / 60) + ':' + pad(Math.round(sec % 60));
    }

    //var log = window.opera ? window.opera.postError : window.console ? window.console.log : function(){};

    function log() {
        return console.log.apply(console, arguments);
    }

    return {
         pad:           pad
        ,log:           log
        ,formatTime:    formatTime
    };
})()

var db = (function()
{
    var subscribers = {};
    var albums = [];
    var artwork = [];

    var sub = function(event, func) {
        if(!subscribers.event) {
            subscribers[event] = [];
        }
        subscribers[event].push(func);
    }
    var fire = function(event) {
        subscribers[event].forEach(function(el) {
            el();
        });
    }
    
    var init = function() {
        $.getJSON('/data/album', function(json) {
            if(json) {
                albums = json;
                fire('albumList');
            }
        });
    }

    var fetchAlbum = function(id, func) {
        if(!albums[id].data) {
            $.getJSON('/data/album/' + albums[id].id, function(json) {
                json.map(function(track) {
                    track.album = id;
                    return track;
                });
                albums[id].data = json;
                func(albums[id]);
            });
        }
        else {
            func(albums[id]);
        }
    }

    var getAlbum = function(id) {
        return albums[id];
    }

    var getAlbums = function(id) {
        return albums;
    }

    var getArt = function(path, callback) {
        if(artwork[path]) {
            callback(artwork[path]);
        }
        else {
            $.getJSON('/data/images/' + path, function(json) {
                artwork[path] = json;
                callback(artwork[path]);
            });
        }
    }

    return {
         fetchAlbum:    fetchAlbum
        ,getAlbum:      getAlbum
        ,getAlbums:     getAlbums
        ,getArt:        getArt
        ,sub:           sub
        ,init:          init
    };
})()

var playlist = (function()
{
    var playlist = [];
    var subscribers = [];
    var pointer = -1;

    function add(data) {
        playlist = playlist.concat(data);
        playlist.map(function(track, index) {
            track.pointer = index;
            return track;
        });
        fire();
    }

    function update(album) {
        playlist = album.data;
        playlist.map(function(track, index) {
            track.pointer = index;
            return track;
        });
        pointer = -1;
        fire();
    }


    function sub(callback) {
        subscribers.push(callback);
    }
    function fire() {
        subscribers.forEach(function(func) {
            func();
        });
    }

    function get() {
        return playlist;
    }

    function next() {
        return playlist[pointer+1] || null;
    }

    function current() {
        return playlist[pointer];
    }

    function change(id) {
        pointer = id-1;
    }

    function increment() {
        pointer++;
    }

    function loadAlbum(id) {
        db.fetchAlbum(id, update)
    }

    return {
         add:           add
        ,get:           get
        ,getCurrent:    current
        ,getNext:       next
        ,change:        change
        ,increment:     increment
        ,subscribe:     sub
        ,loadAlbum:     loadAlbum
    };
})()

var player = (function()
{
    //TODO: * player.play() to change track is quite unintuitive
    //      * merge player code since both share pretty much the same apis and we barely use events
    var subscribers = {};
    var timer;
    var engine;
    var paused = true;

    var init = function() {
        var currentAudio = document.getElementById('html_player');

        if(!!currentAudio.play) {
            // Native <audio> support
            var load = function() {
                currentAudio.pause();
                var id = playlist.getNext().id;
                currentAudio.src = '/resource/' + id + '/asd.ogg';
                util.log("src: " + currentAudio.src);
                currentAudio.load();
            }

            var play = function() {
                load();
                switchTrack();
            }

            var switchTrack = function() {
                load();
                playlist.increment();

                currentAudio.play();
                paused = false;
                fire('paused', paused);

                fire('trackChanged');
            }

            var pause = function() {
                if(paused) {
                    paused = false;
                    currentAudio.play();
                }
                else {
                    paused = true;
                    currentAudio.pause();
                }
                fire('paused', paused);
            }


            var ended = function() {
                switchTrack();
                util.log('ended');
            }

            var pingTime = function() {
                fire('time', currentAudio.currentTime);
            }

            currentAudio.addEventListener('ended', ended, false);
            currentAudio.addEventListener('timeupdate', pingTime, false);

            player = {
                 play: play
                ,pause: pause
                ,sub: subscribe
            }
        }
    }

    var subscribe = function(event, func) {
        if(!subscribers[event])
            subscribers[event] = [];
        subscribers[event].push(func);
    }
    var fire = function(event, arg){
        subscribers[event].forEach(function(func){func(arg)});
    }

    return {
         init:  init
        ,sub:   subscribe
    };
})();

var albumArt = (function()
{
    var img, nav, current;

    var init = function() {
        img = document.getElementById('art_view');
        nav = document.getElementById('art_nav');
        $('#art_nav li').live('click', imgEvent);

        player.sub('trackChanged', load);
    }

    var load = function(id) {
        path = playlist.getCurrent().path_id;
        db.getArt(path, artCallback);
    }

    var artCallback = function(art) {
        changePath(art);
    }

    var changePath = function(art) {
        var active;
        var front = art.filter(function(el) { return (/front/i).test(el.filename) });
        if(front.length > 0) {
            active = front[0].id;
        }
        else if(art.length < 1) {
            active = null;
        }
        else {
            active = art[0].id;
        }

        nav.innerHTML = art.map(function(el){return _s('<li data-id="%s">%s</li>', el.id, el.filename.replace(/.jpg|.png|.gif/i, ''))}).join('');
        changeImg(active);
    }

    var imgEvent = function(e) {
        var id = e.target.getAttribute('data-id');
        changeImg(id);
    }

    var changeImg = function(id) {
        if(current == id) return;

        $('.active', nav).removeClass('active');
        if(id) {
            current = id;
            img.src = "/resource/" + id;
            $('li[data-id=' + id + ']', nav).addClass('active');
        }
        else {
            img.src = '';
        }
    }

    return {
         init: init
    };
})();

var interface = (function()
{
    var albumList;
    var activeTrack;

    function init() {
        player.init();
        player.sub('time', updateTime);
        player.sub('trackChanged', updateMeta);
        player.sub('paused', updateStatus);
        albumArt.init();
        db.sub("albumList", albumsLoaded);
        db.init();

        playlist.subscribe(populatePlaylist);

        $('#album_list > li').live('click', function(e) {
            $(e.target).toggleClass('expanded');
        });
        $('#album_list li li').live('click', function(e) {
            playlist.loadAlbum(e.target.getAttribute('data-id'));
        });
        $('#playlist li[data-id]').live('click', changeTrack);

        attachHandlers();
    }

    function albumsLoaded() {
        var albums = db.getAlbums();
        var artist = '';
        var data = [];

        if(albums[0] && albums[0].sortname === '') {
            data.push('<li>Unknown (no albumartist)<ul>');
        }

        for(var i=0, album; album = albums[i]; ++i) {
            if(album.sortname != artist) {
                artist = album.sortname;
                data.push(_s('</ul></li><li>%s<ul>', album.sortname));
            }
            data.push(_s('<li data-id="%s">%s (%s)</li>', i, album.title, album.date));
        }

        document.getElementById('album_list').innerHTML = data.join('');
    };

    function populatePlaylist() {
        //TODO: Support multiple albums in playlist
        var list = playlist.get();
        var result = [];
        var album = db.getAlbum(list[0].album);

        for(var i=0,track; track  = list[i]; ++i) {
            var artist = album.name != track.name ? track.name + ' - ' : '';
            var text = _s('%s. %s%s<span class="time"><span class="current_time"></span>%s</span>', util.pad(track.tracknumber), artist, track.title, util.formatTime(track.length));
            result.push(_s('<li data-id="%s"><div class="progress_bar"></div><div class="text">%s</div></li>', track.pointer, text));
        }
        
        document.getElementById('album').innerHTML = _s('%s - %s (%s)', album.name, album.title, album.date);
        document.getElementById('playlist').innerHTML = result.join('');
    }

    function changeTrack(e) {
        id = parseInt($(e.target).closest('[data-id]').attr('data-id'));
        playlist.change(id);
        player.play();
        updateMeta();

    }

    function updateMeta() {
        var track = playlist.getCurrent();
        activeTrack = $(_s('#playlist li[data-id=%s]', track.pointer)).get(0);
        $('#playlist li.active').removeClass('active');
        $(activeTrack).addClass('active');
    }
    function updateTime(sec) {
        if(sec < 0) {
            //player.play();
            return;
        }
        var width = $(activeTrack).width();
        $('.current_time', activeTrack).html(util.formatTime(sec));
        $('.progress_bar', activeTrack).css('width', Math.round(sec) / playlist.getCurrent().length * width + 'px');
    }
    function updateStatus(paused) {
        if(paused)
            $('#playlist').addClass('paused');
        else
            $('#playlist').removeClass('paused');
    }

    function attachHandlers() {
        document.addEventListener('keydown', function(e) {
            util.log(e.keyCode);
            if(e.keyCode == 179)
                player.pause();
        }, false);
    }

    $(document).ready(init);
    return null;
})()

function dump(arr,level) {
    var dumped_text = "";
    if(!level) level = 0;
    
    //The padding given at the beginning of the line.
    var level_padding = "";
    for(var j=0;j<level+1;j++) level_padding += "    ";
    
    if(typeof(arr) == 'object') { //Array/Hashes/Objects 
        for(var item in arr) {
            var value = arr[item];
            
            if(typeof(value) == 'object') { //If it is an array,
                dumped_text += level_padding + "'" + item + "' ...\n";
                dumped_text += dump(value,level+1);
            } else {
                dumped_text += level_padding + "'" + item + "' => \"" + value + "\"\n";
            }
        }
    } else { //Stings/Chars/Numbers etc.
        dumped_text = "===>"+arr+"<===("+typeof(arr)+")";
    }
    return dumped_text;
}

function sprintf(format)
{
    for(var i=1; arguments.length > i; i++) {
        format = format.replace('%s', arguments[i]);
    }
    return format;
}
_s = sprintf;

