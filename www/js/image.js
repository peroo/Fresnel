var viewer = (function()
{
    var img = document.getElementById('view');
    var display = function(id) {
        view.src = '/resource/' + id;
    }
    return {
    }
})()

var interface = (function() 
{
    var folders = function(json) {

    }

    var init = function() {
        $.getJSON('/data/images/', folders);
    }

    $(document).ready(init);
})()
