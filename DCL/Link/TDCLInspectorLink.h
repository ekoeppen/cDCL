#ifndef __TDCLINSPECTORLINK__
#define __TDCLINSPECTORLINK__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Link/TDCLLink.h>

// DCL
#include <DCL/Exceptions/TDCLException.h>
#include <DCL/Interfaces/TDCLFSItemRef.h>

class TDCLApplication;

class TDCLInspectorLink : public TDCLLink
{
public:
	TDCLInspectorLink( TDCLApplication* inApplication );
	~TDCLInspectorLink( void );

	virtual void DoConnect( void ) override;
	virtual void Connected( void ) override;

private:
	virtual EState ProcessDockCommand( TDCLDockCommand* inCommand, Boolean* outProcessed ) override;
	virtual EState ProcessAppCommand( TDCLAppCommand* inCommand, Boolean* outProcessed ) override;
};

#endif // __TDCLINSPECTORLINK__
