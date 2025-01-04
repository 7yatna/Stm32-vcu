#ifndef TESLACHARGER_H
#define TESLACHARGER_H

/*  This library supports the various opensource tesla charger controllers e.g. PCS , Gen2/3 etc.

*/

#include "chargerhw.h"
#include "canhardware.h"
#include <stdint.h>
#include "my_math.h"
#include "iomatrix.h"
#include "params.h"					  

class teslaCharger: public Chargerhw
{
public:
    void DecodeCAN(int id, uint32_t data[2]) override;
    void Task100Ms() override;
    bool ControlCharge(bool RunCh, bool ACReq) override;
    void SetCanInterface(CanHardware* c) override;
};

#endif /* TESLACHARGER_H */
