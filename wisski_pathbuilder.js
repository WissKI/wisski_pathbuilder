Drupal.behaviors.wisski_override_autocomplete = function () {

  if (Drupal.jsAC) {

    Drupal.jsAC.prototype.hidePopup = function (keycode) {
      
      // Select item if the right key or mousebutton was pressed
      if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
                
        var id = this.input.id;
        id = id.replace(/^(edit-[-\d]+)(-\d+)$/, "#$1-auth-data$2");
        var hiddenfield = $(id);
        hiddenfield = hiddenfield[0];

        if (/^\[vid:(\d+)\] \[ind:([^\]]+)\] (.*)$/.exec(this.selected.autocompleteValue)) {
          this.input.value = RegExp.$3;
          hiddenfield.value = this.selected.autocompleteValue;

        } else {
          this.input.value = this.selected.autocompleteValue;
        }
        
        

      }
      
//      if(this.selected && keycode == 13) {
//        alert("Da kam enter!");
//        return false;
//      }

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
      return true;
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



  $('.form-item').keydown(function(event) {
    if (event.which == 13) {
      event.stopPropagation();
      event.preventDefault();
      return false;
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

/*  $('.wki-button-dupl').unbind('keypress');
  $('.wki-button-dupl').unbind('keydown');
  $('.wki-button-dupl').bind('keypress', (function(event){
      alert("haha");

    if (event.keyCode == 10 || event.keyCode == 13) { 
      alert("haha");
      return false;
    }
  }));*/
  
  
});

