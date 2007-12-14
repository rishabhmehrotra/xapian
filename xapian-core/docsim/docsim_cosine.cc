/* docsim_cosine.cc: cosine similarity
 *
 * Copyright 2007 Yung-chung Lin
 * Copyright 2007 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <config.h>

#include <xapian/cluster.h>

#include "omdebug.h"

#include <map>
#include <math.h>

Xapian::DocSimCosine::~DocSimCosine()
{
    DEBUGAPICALL(void, "DocSimCosine::~DocSimCosine", "");
}

double
Xapian::DocSimCosine::similarity(TermIterator a_begin,
				 const TermIterator & a_end,
				 TermIterator b_begin,
				 const TermIterator & b_end) const
{
    DEBUGAPICALL(void, "DocSimCosine::~calculate_similarity",
		 a_begin << ", " << a_end << ", " <<
		 b_begin << ", " << b_end);
    DummyTermFreqSource dummytfs;
    const TermFreqSource * tfs = ((freqsource == NULL) ? &dummytfs : freqsource);

    Xapian::doccount doc_count = tfs->get_doccount();

    std::map<std::string, double> wt_a;
    std::map<std::string, double> wt_b;

    double wt_a_denom = 0;
    double wt_b_denom = 0;

    Xapian::TermIterator titer;

    for (titer = a_begin; titer != a_end; ++titer) {
	double tf;
	tf = tfs->get_termfreq(*titer);
	if (tf < 1.0) tf = 1.0;
	double idf = log(doc_count / tf);
	double wdf = titer.get_wdf();
	if (wdf < 1) wdf = 1;
	double tmp = (log(wdf) + 1.0) * idf;
	wt_a[*titer] = tmp;
	wt_a_denom += tmp * tmp;
    }

    for (titer = b_begin; titer != b_end; ++titer) {
	double tf;
	tf = tfs->get_termfreq(*titer);
	if (tf < 1.0) tf = 1.0;
	double idf = log(doc_count / tf);
	double wdf = titer.get_wdf();
	if (wdf < 1) wdf = 1;
	double tmp = (log(wdf) + 1.0) * idf;
	wt_b[*titer] = tmp;
	wt_b_denom += tmp * tmp;
    }

    if (wt_a_denom == 0 || wt_b_denom == 0) {
	// At least one of the lists is entirely composed of zero weights.
	RETURN(0.0);
    }

    std::map<std::string, double>::iterator wt_iter;

    for (wt_iter = wt_a.begin(); wt_iter != wt_a.end(); ++wt_iter) {
	wt_iter->second /= wt_a_denom;
    }

    for (wt_iter = wt_b.begin(); wt_iter != wt_b.end(); ++wt_iter) {
	wt_iter->second /= wt_b_denom;
    }

    double wt_sq_sum_a = 0;
    double wt_sq_sum_b = 0;
    double inner_product = 0;

    for (wt_iter = wt_a.begin(); wt_iter != wt_a.end(); ++wt_iter) {
	wt_sq_sum_a += wt_iter->second * wt_iter->second;
	std::map<std::string, double>::iterator wt_iter2;
	wt_iter2 = wt_b.find(wt_iter->first);
	if (wt_iter2 != wt_b.end()) {
	    inner_product += wt_iter->second * wt_iter2->second;
	}
    }

    for (wt_iter = wt_b.begin(); wt_iter != wt_b.end(); ++wt_iter) {
	wt_sq_sum_b += wt_iter->second * wt_iter->second;
    }

    RETURN(inner_product / (sqrt(wt_sq_sum_a) * sqrt(wt_sq_sum_b)));
}

std::string
Xapian::DocSimCosine::get_description() const
{
    DEBUGCALL(INTRO, string, "Xapian::DocSimCosine::get_description", "");
    RETURN("Xapian::DocSimCosine()");
}
