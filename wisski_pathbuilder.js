Drupal.behaviors.wisski_override_autocomplete = function () {

  if (Drupal.jsAC) {
    
    Drupal.jsAC.prototype.hidePopup = function (keycode) {
      // Select item if the right key or mousebutton was pressed
      if (this.selected && ((keycode && keycode != 46 && keycode != 8 && keycode != 27) || !keycode)) {
        
        var id = this.input.id;
        id = id.replace(/^edit-/, "#edit-auth-data-");
        var hiddenfield = $(id);
        hiddenfield = hiddenfield[0];

        if (/^\[vid:(\d+)\] \[ind:([^\]]+)\] (.*)$/.exec(this.selected.autocompleteValue)) {
          this.input.value = RegExp.$3;
          hiddenfield.value = this.selected.autocompleteValue;

        }

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
    };

  }
};

