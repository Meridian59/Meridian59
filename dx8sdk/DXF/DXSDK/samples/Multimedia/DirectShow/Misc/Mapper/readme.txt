DirectShow Sample -- Mapper
---------------------------

This sample demonstrates using the IFilterMapper2::EnumMatchingFilters method 
to locate filters in the registry. The application's user interface contains controls 
that match the parameters of the EnumMatchingFilters method, such as minimum merit value, 
pin categories, and media types. 

Select the search options that you want, then click the Search button to enumerate 
the filters that match those options.  The list box will display the results of your
query, including the filter's Friendly Name, Filter merit, and the filename of the
server which contains it.  Tooltips are provided to help explain the user interface
elements.  

Although the EnumMatchingFilters method allows you to specify the number of 
media type/subtype pairs to use in the query, the Mapper application only provides
two sets of list boxes for input and output types.  If the first media type is specified
as a "Don't Care" condition, then the secondary media type/subtype listboxes will
be disabled to prevent confusion.  By default, the search elements are configured
to display all filters with a merit value of Normal or higher.
