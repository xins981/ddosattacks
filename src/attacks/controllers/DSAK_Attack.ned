package ddosattacks.attacks.controllers;

simple DSAK_Attack
{
    parameters:
        @display("i=misc/cloud3,red,100;is=l");
        string attackType @enum("dropping","sinkhole","delay") = default("");  //String with the type of attack
        bool active = default(false);  //Indicates if this attack is active or not in the node
        double startTime @unit("s") = default(0s); // Timestamp to start the attack
        double endTime @unit("s") = default(0s); //Time instant at which the attack ends, 0 means that the attack will not end during the simulation.
}
