#ifndef __DSAK_TCPRENO_H
#define __DSAK_TCPRENO_H

#include "TcpReno.h"

namespace dsak
{
    namespace tcp
    {
        class DSAK_TcpReno : public inet::tcp::TcpReno
        {
            public:
                virtual void established(bool active) override;
        };
    }
}

#endif // ifndef __DSAK_TCPRENO_H

