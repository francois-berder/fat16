/*
 * Copyright (C) 2017  Francois Berder <fberder@outlook.fr>
 *
 * This file is part of fat16.
 *
 * fat16 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fat16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fat16.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _TEST_HPP_
#define _TEST_HPP_

#include <string>

class Test
{
    public :

        Test(const std::string &name);
        virtual ~Test() = default;

        virtual void init();
        virtual bool run() = 0;
        virtual void release();

        const std::string get_name() const;

    private :

        const std::string m_name;
};

#endif
