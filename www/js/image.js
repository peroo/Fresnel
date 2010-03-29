var viewer = (function()
{
    var images = [];
    var current;

    var img = document.getElementById('view');
    var display = function(index) {
        if(index >= images.length) return;
        view.src = '/resource/' + images[index].id;
        $('#filename').html(images[index].filename);
        $('#count').text(index+1 + '/' + images.length);
        current = index;
    }

    var init = function(json) {
        images = json;
        images.sort(function(a,b) {
            if(a.filename > b.filename) return 1;
            else if(a.filename < b.filename) return -1;
            else return 0;
        });
        var i=0; 
        while(i < images.length)
            display(i++);
        display(0);
    }

    var prev = function() {
        if(current > 0)
            display(current-1);
    };

    var next = function() {
        if(images.length > 0 && current < images.length)
            display(current+1);
    };

    return {
        init: init
        ,next: next
        ,prev: prev
    }
})()

var db = (function()
{
    var folders = [];
    var events = [];
    var parents = [];

    var init = function() {
        $.getJSON('/data/path', function(json) {
            json.forEach(function(path) {
                if(!folders[path.parent])
                    folders[path.parent] = [];

                folders[path.parent].push(path);
                parents[path.path_id] = path.parent;
            });
            events['folders'].forEach(function(func) {
                func();
            });
        });
    }

    var sub = function(event, func) {
        if(!events[event])
            events[event] = [];

        events[event].push(func);
    }

    var get = function(path) {
        return folders[path];
    }

    var getParent = function(folder) {
        return parents[folder];
    }

    return {
         init: init
        ,get: get
        ,getParent: getParent
        ,subscribe: sub
    }
})()

var interface = (function() 
{
    var renderFolder = function(folder) {
        folder = folder || 0;
        $.getJSON('/data/image/path/' + folder, viewer.init);

        var folders = db.get(folder) || [];
        folders.sort(function(a,b) {
            if(a.path > b.path) return 1;
            else if(a.path < b.path) return -1;
            else return 0;
        });

        folders = folders.map(function(folder) {
            return '<li data-id="' + folder.path_id + '">' + folder.path + '</li>';
        });
        folders = ['<li data-id="' + db.getParent(folder) + '">..</li>'].concat(folders);

        $('#folders').html(folders.join(''));
    }

    var init = function() {
        db.subscribe('folders', renderFolder);
        db.init();

        $('#folders').click(function(e) {
            if(e.target.tagName == 'LI') {
                renderFolder(e.target.getAttribute('data-id'));
            }
        });
        $('#prev_img').click(viewer.prev);
        $('#next_img').click(viewer.next);
        $(document).bind('keydown', function(e) {
            if(e.keyCode == 32)
                viewer.next();
        });
    }

    $(document).ready(init);
})()
