/* Homepage http://facetracknoir.sourceforge.net/home/default.htm
 *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#include "csv.h"
#include "compat/macros.hpp"
#include "compat/library-path.hpp"

#include <ios>
#include <fstream>
#include <sstream>
#include <vector>

#include <QString>
#include <QDebug>

static bool
check_line(int id, unsigned char* table, QString& game_name, const std::vector<std::string>& fields, unsigned lineno)
{
    for (int i = 0; i < 8; i++)
        table[i] = 0;

    if (id != 0)
        qDebug() << "csv: lookup game id" << id;

    const auto id_str = std::to_string(id);

    unsigned tmp[8];
    unsigned fuzz[3];

    //qDebug() << "Column 0: " << gameLine.at(0);		// No.
    //qDebug() << "Column 1: " << gameLine.at(1);		// Game Name
    //qDebug() << "Column 2: " << gameLine.at(2);		// Game Protocol
    //qDebug() << "Column 3: " << gameLine.at(3);		// Supported since version
    //qDebug() << "Column 4: " << gameLine.at(4);		// Verified
    //qDebug() << "Column 5: " << gameLine.at(5);		// By
    //qDebug() << "Column 6: " << gameLine.at(6);		// International ID
    //qDebug() << "Column 7: " << gameLine.at(7);		// FaceTrackNoIR ID

    if (fields.size() == 8)
    {
        if (fields[6] == id_str)
        {
            const auto& proto = fields[3];
            const auto& name = fields[1];

            const auto& proto_id = fields[7];

            auto do_scanf = [&]() {
              return sscanf(proto_id.c_str(),
                            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                            fuzz + 2,
                            fuzz + 0,
                            tmp + 3, tmp + 2, tmp + 1, tmp + 0,
                            tmp + 7, tmp + 6, tmp + 5, tmp + 4,
                            fuzz + 1);
            };

            if (proto == "V160" || proto_id.size() != 22)
                (void)0;
            else if (proto_id.size() != 22 || do_scanf() != 11)
                qDebug() << "scanf failed" << lineno;
            else
                for (int i = 0; i < 8; i++)
                    table[i] = (unsigned char)tmp[i];
            game_name = QString::fromStdString(name);
            return true;
        }
    }
    else
        eval_once(qDebug() << "malformed csv line" << lineno);

    if (id)
        qDebug() << "unknown game connected" << id;

    return false;
}

bool get_game_data(int id, unsigned char* table, QString& game_name)
{
    static const auto filename =
        (OPENTRACK_BASE_PATH + OPENTRACK_DOC_PATH "settings/facetracknoir supported games.csv").toStdWString();
    std::ifstream in;
    in.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	in.open(filename);
    try
    {
        if (in.fail())
            eval_once(qDebug() << "can't open csv file");

        std::string line, field;
        line.reserve(1024); field.reserve(1024);
        std::vector<std::string> fields; fields.reserve(16);
        unsigned lineno = 0;
        try
        {
            while (!in.eof())
            {
                line.clear();
                std::getline(in, line);
                std::stringstream s{line, std::ios_base::in};
                fields.clear();
                while (!s.eof())
                {
                    field.clear();
                    std::getline(s, field, ';');
                    fields.push_back(field);
                }
                bool ret = check_line(id, table, game_name, fields, lineno);
                if (ret)
                    return true;
                lineno++;
            }
        }
        catch (const std::ios::failure& e)
        {
            eval_once(qDebug() << "failed to read .csv file:" << e.what());
        }
    }
    catch (const std::ios::failure& e)
    {
        eval_once(qDebug() << "can't open .csv file:" << e.what());
    }

    return false;
}
