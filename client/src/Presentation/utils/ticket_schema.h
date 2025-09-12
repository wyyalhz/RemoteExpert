#ifndef TICKETSCHEMA_H
#define TICKETSCHEMA_H
#include <QtSql>

inline void ensureTicketSchema(){
    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS tickets("
           "sid INTEGER PRIMARY KEY,"
           "factory TEXT,"
           "expert TEXT,"
           "status TEXT,"
           "title TEXT,"
           "description TEXT"
           ")");
}
#endif // TICKETSCHEMA_H
