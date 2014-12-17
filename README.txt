CPSC 426: Building Decentralized Systems Final Project README Final Group
Version Fall 2014

Group: Kayo Teramoto, Charles Jin, Daniel Chiu

Last Updated: 12/15/2014
-------------------------------------------------------------------------------
BRIEF DESCRIPTION: We implemented a distributed hash table and a vanishing
secret sharing system on top of our existing peerster.

Our project is based on Sharmir’s Secret Sharing algorithm (1979), Chord (2001),
and Vanish (2009).


HOW TO RUN THE SYSTEM: Run two or more peerster instances.
> ./peerster
OR
> ./peerster <hostname:portnumber>

Have the peerster nodes add each other so that they all have each other as a
peer. (Note: this may be done from the command line (see above) or by clicking
on the “Add Peer” button and entering peer information.

Once all of the peers have connected with each other, we can click on the “Show
Finger Table” button to see the finger table. Nodes automatically join the DHT
when they join the network. Click on the button again to close the table.

To share a secret, click on the “Share Secret” button and enter a message. This
message is encrypted and sent to all other peers in the network. The encryption
key is divided and distributed across the peers in the network as well.

Any node in the network can reconstruct a message. Click on the “Reconstruct
Secret” button to see the list of secrets that have been distributed to that
given node. Click on a secret to request reconstruction of the secret. This
sends a message to all of the peers in the network requesting their share of the
encryption key. If more than some threshold number of shares are returned to the
requesting node, the encryption key can be reconstructed and the secret
decrypted. The message will be displayed in the line following “The recovered
secret.” If the secret cannot be reconstructed (due the number of encryption key
shares in the network falling below the required threshold from churn), no
message will be displayed. Closing the dialog destroys the information until the
secret is requested again (so that reconstructed messages are not saved).

The system is currently set to have its threshold be 3/4 of the number of total
nodes connected with everyone participating. This was done for testing purposes:
starting up four nodes and removing one still leaves the secret reconstructable,
but deleting a second node renders the secret undecrytable. In a more realistic
scheme, it would be harder to see the actually "vanishing" nature of the data
behavior correctly without multiple tests and a large number of connected nodes
due to the randomized nature.

Disclaimer: Secrets must be entered before nodes are terminatd. The threhold
is established based on the peerster routing table size, which unfortunately
does not automatically update when nodes die. The DHT however will update
and this update can be seen in the finger table.

FOR MORE INFORMATION: Please see CPSC426_FinalProjectSpec.txt (in this
directory) or contact Kayo (kayo.teramoto@yale.edu), Charles
(charles.jin@yale.edu), or Daniel (daniel.chiu@yale.edu).
