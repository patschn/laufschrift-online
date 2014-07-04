'use strict';
(function ($) {
    $.widget('swp.listasselect', {
        
        // Standarwerte
        options: {
        },
                
        _create: function() {
            this._on({ 'click li': this._clickHandler, 'dblclick li': this._dblclickHandler });
        },
        
        _select: function(elem) {
            this.element.children('li').removeAttr('selected');
            elem.attr('selected', 'selected');
        },
        
        _clickHandler: function(event) {
            var target = $(event.target);
            this._select(target);      
            this._trigger('click', event, target.data('value'));
        },
        
        _dblclickHandler: function(event) {
            var target = $(event.target);
            this._trigger('dblclick', event, target.data('value'));
        },
        
        _elemForValue: function(value) {
            return this.element.children('li[data-value="' + value + '"]');
        },
        
        getSelectedElement: function() {
            return this.element.children('li[selected]:first');
        },
        
        getSelectedValue: function() {
            return this.getSelectedElement().data('value');
        },
        
        removeElementByValue: function(value) {
            this._elemForValue(value).remove();
        },
        
        appendElement: function(value, text) {
            $('<li/>').text(text).attr('data-value', value).appendTo(this.element);
        },
        
        selectElementByValue: function(value) {
            this._select(this._elemForValue(value));
        }
    
    });
})(jQuery);
