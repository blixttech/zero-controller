#pragma once

#include "smp.hpp"
#include "header.hpp"
#include "request.hpp"
#include "response.hpp"

namespace smp {


enum OsMgmtId
{
	ID_ECHO           = 0,
	ID_CONS_ECHO_CTRL = 1,
	ID_TASKSTAT       = 2,
	ID_MPSTAT         = 3,
	ID_DATETIME_STR   = 4,
	ID_RESET          = 5
};


class ResetReq : public Request
{
public:
    ResetReq(bool force = false);

    void serialize(QByteArray& writer) override;
private:
		bool force;
};


class ResetResp : public Response
{
public:
    ResetResp(const Header& header, const QByteArray& payload);

    virtual bool deserialize(const QByteArray& payload);

		int rc() const { return m_rc; }

private:
		int m_rc;
};

} // end namespace