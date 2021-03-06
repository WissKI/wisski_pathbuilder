Drupal.behaviors.wisski_override_autocomplete = function () {

  if (Drupal.jsAC) {

    Drupal.jsAC.prototype.hidePopup = function (keycode) {
      
      // Select item if the right key or mousebutton was pressed
      if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
                        
        var id = this.input.id;
        id = id.replace(/^(edit-[-\d]+)(-\d+)$/, "#$1-auth-data$2");
        var hiddenfield = $(id);
        hiddenfield = hiddenfield[0];

//        if (/^\[vid:(\d+)\] \[ind:([^\]]+)\] (.*)$/.exec(this.selected.autocompleteValue)) {
//          this.input.value = RegExp.$3;
        if (/^\[vid:(\d+)\] \[ind:([^\]]+)\] \[x:([^\]]+)\] (.*)$/.exec(this.selected.autocompleteValue)) {
          this.input.value = RegExp.$4;
          hiddenfield.value = this.selected.autocompleteValue;
          

        } else {
          this.input.value = this.selected.autocompleteValue;
        }
        
        

      }
      
      if(this.selected && keycode == 13) {
      }

      // Hide popup
      var popup = this.popup;
      if (popup) {
        this.popup = null;
        $(popup).fadeOut('fast', function() {
            $(popup).remove();
        });
        // Add-on for OpenLayer Geocoder module
        //        if ($(this.input).attr('geoautocomplete')) {
        //          geocoder = new Drupal.Geocoder(this);
        //          geocoder.process(this.input.value);
        //        }
      }

      this.selected = false;      
      return false;
    };

  }
};


/*
Drupal.behaviors.correct_autocomplete_select = function () {
  
  if(Drupal.jsAC) {
    
  }
}
*/


$(document).ready(function(){
  /*
  $('form').bind("keydown", function(e) {
    var code = e.keyCode || e.which; 
    if (code  == 13) {
          alert("don't!");               
       e.preventDefault();
       return false;
    }
  });
  */
  
  var jQueryVersion = jQuery.prototype.jquery.split(".");
  
  if(jQueryVersion[0] <= 1 && jQueryVersion[1] < 7) {
  
    $('.form-item input').live("keydown", function(event) {
      if (event.which == 13) {
        event.preventDefault();
      }
    });
  } else {
    $('.form-item input').on("keydown", function(event) {
      if (event.which == 13) {
        event.preventDefault();
      }
    });
  }
  
  /*
  $('.form-item input').keydown(function(event) {
    if (event.which == 13) {
//      alert("stop!");
//      Drupal.jsAC.prototype.hidePopup(event.keyCode);
      event.preventDefault();
      //event.stopPropagation();
      //event.preventDefault();
//~      alert("stopped!");
//      return false;
//      alert("haha!");
//      event.preventDefault();
//      event.stopPropagation();
//      return false;
//      $("#autocomplete").hidePopup(event.keyCode);
//      return false;
//      this.click();
//      event.preventDefault();
//      return false;
//  if ($("#autocomplete").length == 0) {
    }
  });
  */
  
});

