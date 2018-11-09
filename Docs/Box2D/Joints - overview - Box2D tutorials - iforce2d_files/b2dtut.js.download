
jQuery(document).ready(function() {
    
    jQuery("#viewsource_link").click( function(e) {
        e.preventDefault();
        jQuery("#viewsource_content").slideToggle("slow");
    });
    
    jQuery(".toggleDivLink").click( function(e) {
        e.preventDefault();
        var divId = jQuery(this).attr("divid");
        jQuery("#"+divId).slideToggle("slow");
    });
    
    jQuery(".toggleDivLinkAll").click( function(e) {
        e.preventDefault();
        var divId = jQuery(this).attr("divid");
        console.log("divid "+divId);
        jQuery("."+divId).slideToggle("slow");
    });
    
    //if ( typeof(startHeader) != 'undefined' )
    //    startHeader();
    
});
