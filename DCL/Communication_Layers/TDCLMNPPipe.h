// ==============================
// Fichier:			TDCLMNPPipe.h
// Projet:			Desktop Connection Library
//
// Créé le:			27/01/2002
// Tabulation:		4 espaces
//
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is TDCLMNPPipe.h.
//
// The Initial Developers of the Original Code are Paul Guyot, Michael Vacík,
// Filip Zawadiak and Nicolas Zinovieff. Portions created by the Initial
// Developers are Copyright (C) 1998-2004 the Initial Developers. All Rights
// Reserved.
//
// Contributor(s):
//   Paul Guyot <pguyot@kallisys.net> (original author)
//   Michael Vacík <mici@metastasis.net> (original author)
//   Filip Zawadiak <philz@vyx.net> (original author)
//   Nicolas Zinovieff <krugazor@poulet.org> (original author)
//
// ***** END LICENSE BLOCK *****
//
// Copyright (c) 2001 Filip Zawadiak <philz@vyx.net>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//   derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ===========
// $Id: TDCLMNPPipe.h,v 1.3 2004/11/24 14:08:53 paul Exp $
// ===========

#ifndef __TDCLMNPPIPE__
#define __TDCLMNPPIPE__

#include <DCL/Headers/DCLDefinitions.h>
#include <DCL/Communication_Layers/TDCLPipeProxy.h>

// DCL
#include <DCL/Exceptions/IO_Exceptions/TDCLIOException.h>
#include <chrono>
#include <vector>

/// The sole error reported for an MNP protocol failure or deadline expiry.
class TDCLMNPProtocolError : public TDCLIOException
{
public:
    TDCLMNPProtocolError() : TDCLIOException(TDCLException::exUnknownError, 0
#if kDCLDebugFILEandLINE
        , __FILE__, __LINE__
#endif
        ) {}
    const char* GetExceptionName() const override { return "protocol error, link dropped"; }
};

///
/// Une connexion pour la compression MNP. S'utilise pour la compression MNP
/// sur liaison série.
///
/// \author	Paul Guyot <pguyot@kallisys.net>
/// \author	Filip Zawadiak <philz@vyx.net> (?)
/// \version $Revision: 1.3 $
///
/// Simplified one-frame, stop-and-wait MNP transport for the Newton profile.
/// \test	MNPPipeTest exercises the wire-level transport with an in-memory peer.
///
class TDCLMNPPipe
	:
		public TDCLPipeProxy
{
public:
    static constexpr std::chrono::milliseconds kDefaultStallTimeout{6000};

	///
	/// Constantes pour cette classe.
	///
	enum {
		kDefaultOutputMaxBufferSize	= 253,	///< 256 - 3.
		///< Le code de microcom donne 64+5. Philz donne 2K
		///< cependant il n'a pas de paquets plus gros que 256 (il tronçonne
		///< le paquet Newton à installer en morceaux de 128 octets). Le Newton
		///< n'aime pas mes paquets de 299 octets, en revanche, ça marche
		///< avec 253+3.
	};

	///
	/// Constructeur à partir d'une couche de communication et d'une taille
	/// maximale de mémoire tampon de sortie.
	///
	/// \param inSubPipe				couche de communication sur laquelle
	///									faire de la compression MNP.
	/// \param inOutputMaxBufferSize	taille maximale de la mémoire tampon
	///									de sortie.
	///
	TDCLMNPPipe(
			TDCLPipe* inSubPipe,
			KUInt32 inOutputMaxBufferSize = kDefaultOutputMaxBufferSize );

	///
	/// Destructeur.
	///
	virtual ~TDCLMNPPipe( void );

    /// Default is 6 s stall wait across handshake, ACK, credit and peer LD.
    /// Shorter deadlines may be configured for deterministic test peers.
    void SetStallTimeout(std::chrono::milliseconds timeout);
    std::chrono::milliseconds GetStallTimeout() const { return mStallTimeout; }

	/// \name interface entrée/sortie

	///
	/// Lit des octets.
	///
	/// \param outBuffer	mémoire tampon pour les octets lus.
	/// \param ioCount		nombre d'octets à lire en entrée, lus en sortie.
	///						Cette valeur est mise à jour avant que
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLIOException	si un problème est survenu (autre que EOF)
	///
	virtual	void		Read( void* outBuffer, KUInt32* ioCount );

	///
	/// Ecrit des octets.
	///
	/// \param inBuffer		mémoire tampon pour les octets à écrire.
	/// \param ioCount		nombre d'octets à écrire en entrée, écris en
	///						sortie. Cette valeur est mise à jour avant que
	///						l'exception ne soit lancée si un problème est
	///						survenu.
	/// \throws TDCLIOException	si un problème est survenu.
	///
	virtual	void		Write( const void* inBuffer, KUInt32* ioCount );

	///
	/// Vide la mémoire tampon de sortie.
	///
	/// \throws TDCLIOException	si un problème est survenu.
	///
	virtual void		FlushOutput( void );

	///
	/// Détermine si des octets sont disponibles dans la mémoire tampon d'entrée.
	///
	/// \return \c true s'il y a des octets disponibles, \c false sinon.
	///
	virtual	Boolean		BytesAvailable( void );

	///
	/// Déconnecte le canal de communication avec le client.
	///
	virtual	void		Disconnect( void );

	///
	/// Whether the Newton closed the MNP link with an LD frame.
	///
	virtual	Boolean		PeerDisconnected( void );

	///
	/// Méthode appelée par le lien pour dire qu'il est connecté
	/// via cette connexion. Cette méthode est appelée juste avant
	/// que des données soient échangées.
	///
	/// Envoie l'entête MNP (et reçoit celui du Newton).
	///
	/// \return l'interface de communication.
	///
	virtual	TDCLCommLayer*	Connected( TDCLLink* inLink );

	///
	/// Send ACK
	///
	virtual	void		SendACK( void );

	///
	/// Send ACK
	///
	inline	void		SetKeepAlive( Boolean inKeepAlive )
		{
            mKeepAlive = inKeepAlive;
		}

    /// Peer LD reason; zero if none was received.
    KUInt8 PeerDisconnectReason() const { return mPeerReason; }
    /// Whether local disconnect completed after the final LT was acknowledged.
    Boolean DisconnectedCleanly() const { return mCleanDisconnect; }
    /// Reason sent in the local LD (zero if none was sent).
    KUInt8 DisconnectReason() const { return mDisconnectReason; }

	///
	/// Direction of a logged MNP frame.
	///
	enum EFrameDirection {
		kFrameIn,		///< Frame received from the Newton.
		kFrameOut,		///< Frame sent to the Newton.
		kFrameError		///< Framing/CRC error while receiving (partial data).
	};

	///
	/// Callback used to trace MNP frames (decoded payload, without
	/// SYN/DLE/STX framing, DLE stuffing and CRC).
	///
	typedef void (*FrameLoggerPtr)(
					void* inRefCon,
					EFrameDirection inDirection,
					const KUInt8* inFrame,
					KUInt32 inSize );

	///
	/// Install a process-wide MNP frame logger (nil to disable).
	///
	static	void		SetFrameLogger( FrameLoggerPtr inLogger, void* inRefCon );

private:
	///
	/// Constructeur par copie volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMNPPipe( const TDCLMNPPipe& inCopy );

	///
	/// Opérateur d'assignation volontairement indisponible.
	///
	/// \param inCopy		objet à copier
	///
	TDCLMNPPipe& operator = ( const TDCLMNPPipe& inCopy );

	///
	/// Forward a frame to the installed logger, if any.
	///
	static	void		LogFrame( EFrameDirection inDirection,
							const KUInt8* inFrame, KUInt32 inSize );

    using Clock = std::chrono::steady_clock;
    using Bytes = std::vector<KUInt8>;

    void SendFrame(const Bytes& frame);
    Bytes ReceiveFrame(Clock::time_point deadline, KUInt8 timeoutReason);
    void Pump(Clock::time_point deadline, KUInt8 timeoutReason = 0x04);
    void DrainAvailable();
    void SendData();
    template <typename Predicate>
    void WaitFor(Predicate done, KUInt8 timeoutReason = 0x04);
    [[noreturn]] void Fail(KUInt8 reason = 0xfe);

    KUInt32 mOutputCapacity;
    Bytes mOutput;
    Bytes mInput;
    size_t mInputOffset = 0;
    KUInt8 mSendSequence = 0;
    KUInt8 mReceiveSequence = 0;
    KUInt8 mCredit = 0;
    KUInt32 mProgress = 0;
    bool mPending = false;
    bool mHaveInbound = false;
    bool mConnected = false;
    bool mClosed = false;
    bool mPeerClosed = false;
    bool mKeepAlive = false;
    bool mCleanDisconnect = false;
    KUInt8 mPeerReason = 0;
    KUInt8 mDisconnectReason = 0;
    std::chrono::milliseconds mStallTimeout{kDefaultStallTimeout};

	static FrameLoggerPtr	sFrameLogger;		///< Frame logger callback.
	static void*			sFrameLoggerRefCon;	///< Frame logger refcon.
};

#endif
		// __TDCLMNPPIPE__

// ====================================================================== //
// C makes it easy for you to shoot yourself in the foot.  C++ makes that //
// harder, but when you do, it blows away your whole leg.                 //
//                 -- Bjarne Stroustrup                                   //
// ====================================================================== //
