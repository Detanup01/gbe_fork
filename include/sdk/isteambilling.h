
#ifndef ISTEAMBILLING_H
#define ISTEAMBILLING_H

// this interface is not found in public SDK archives, it is based on reversing the returned vftable from steamclient64.dll

class ISteamBilling
{
public:
    virtual bool _unknown_fn_1( ) = 0;
    virtual bool _unknown_fn_2( ) = 0;
    virtual bool _unknown_fn_3( ) = 0;
    virtual bool _unknown_fn_4( ) = 0;
    virtual bool _unknown_fn_5( ) = 0;
    virtual bool _unknown_fn_6( ) = 0;
    virtual bool _unknown_fn_7( ) = 0;
    virtual bool _unknown_fn_8( ) = 0;
    virtual bool _unknown_fn_9( ) = 0;
    virtual bool _unknown_fn_10( ) = 0;
    virtual bool _unknown_fn_11( ) = 0;
    virtual bool _unknown_fn_12( ) = 0;
    virtual bool _unknown_fn_13( ) = 0;
    virtual bool _unknown_fn_14( ) = 0;
    virtual bool _unknown_fn_15( ) = 0;
    virtual bool _unknown_fn_16( ) = 0;
    virtual bool _unknown_fn_17( ) = 0;
    virtual bool _unknown_fn_18( ) = 0;
    virtual bool _unknown_fn_19( ) = 0;

    virtual int _unknown_fn_20( ) = 0;
    virtual int _unknown_fn_21( ) = 0;
    virtual int _unknown_fn_22( ) = 0;
    virtual int _unknown_fn_23( ) = 0;
    virtual int _unknown_fn_24( ) = 0;
    virtual int _unknown_fn_25( ) = 0;
    virtual int _unknown_fn_26( ) = 0;

    virtual const char* _unknown_fn_27( ) = 0; // returns null string (str address is inside .rdata so it can't change at runtime)

    virtual int _unknown_fn_28( ) = 0;

    virtual int _unknown_fn_29( ) = 0; // mov eax, 2

    virtual int _unknown_fn_30( ) = 0;
    virtual int _unknown_fn_31( ) = 0;
    virtual int _unknown_fn_32( ) = 0;
    virtual int _unknown_fn_33( ) = 0;
    virtual int _unknown_fn_34( ) = 0;
    virtual int _unknown_fn_35( ) = 0;
    virtual int _unknown_fn_36( ) = 0;
    virtual int _unknown_fn_37( ) = 0;

    virtual const char* _unknown_fn_38( ) = 0; // returns null string (str address is inside .rdata so it can't change at runtime)

    virtual int _unknown_fn_39( ) = 0;
    virtual int _unknown_fn_40( ) = 0;

    virtual bool _unknown_fn_41( ) = 0;
    virtual bool _unknown_fn_42( ) = 0;
    virtual bool _unknown_fn_43( ) = 0;
};

#define STEAMBILLING_INTERFACE_VERSION "SteamBilling002"

#endif // ISTEAMBILLING_H
