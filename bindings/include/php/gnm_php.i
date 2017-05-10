/*
 * $Id: gnm_php.i 34525 2016-07-03 02:53:47Z goatbar $
 *
 * php specific code for gnm bindings.
 */

%init %{
  if ( OGRGetDriverCount() == 0 ) {
    OGRRegisterAll();
  }
%}

%include typemaps_php.i
