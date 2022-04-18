#include "DSAK_TcpReno.h"

Register_Class(dsak::tcp::DSAK_TcpReno);

namespace dsak
{
    namespace tcp
    {
        void DSAK_TcpReno::established(bool active)
        {
            if (state->increased_IW_enabled && state->syn_rexmit_count == 0)
            {
                state->snd_cwnd = std::min(4 * state->snd_mss, std::max(2 * state->snd_mss, (inet::uint32)4380));
                EV_DETAIL << "Enabled Increased Initial Window, CWND is set to " << state->snd_cwnd << "\n";
            }
            else
            {
                state->snd_cwnd = state->snd_mss; // RFC 2001
            }

            if (active)
            {
                // finish connection setup with ACK (possibly piggybacked on data)
                EV_INFO << "Completing connection setup by sending ACK (possibly piggybacked on data)\n";
                if (!sendData(false)) // FIXME TODO - This condition is never true because the buffer is empty (at this time) therefore the first ACK is never piggyback on data
                {
                    //conn->sendAck();
                    EV_DETAIL << "don't send ack" << "\n";
                }
            }
        }
    }
}



