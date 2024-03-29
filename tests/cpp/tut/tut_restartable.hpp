#ifndef TUT_RESTARTABLE_H_GUARD
#define TUT_RESTARTABLE_H_GUARD

#include <tut/tut.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

/**
 * Optional restartable wrapper for test_runner.
 *
 * Allows to restart test runs finished due to abnormal
 * test application termination (such as segmentation
 * fault or math error).
 *
 * @author Vladimir Dyuzhev, Vladimir.Dyuzhev@gmail.com
 */

namespace tut
{

namespace util
{

/**
 * Escapes non-alphabetical characters in string.
 */
std::string escape(const std::string &orig)
{
    std::string rc;
    std::string::const_iterator i, e;
    i = orig.begin();
    e = orig.end();

    while (i != e)
    {
        if ((*i >= 'a' && *i <= 'z') || (*i >= 'A' && *i <= 'Z') ||
            (*i >= '0' && *i <= '9'))
        {
            rc += *i;
        }
        else
        {
            rc += '\\';
            rc += ('a' + (((unsigned int)*i) >> 4));
            rc += ('a' + (((unsigned int)*i) & 0xF));
        }

        ++i;
    }
    return rc;
}

/**
 * Un-escapes string.
 */
std::string unescape(const std::string &orig)
{
    std::string rc;
    std::string::const_iterator i, e;
    i = orig.begin();
    e = orig.end();

    while (i != e)
    {
        if (*i != '\\')
        {
            rc += *i;
        }
        else
        {
            ++i;
            if (i == e)
            {
                throw std::invalid_argument("unexpected end of string");
            }
            unsigned int c1 = *i;
            ++i;
            if (i == e)
            {
                throw std::invalid_argument("unexpected end of string");
            }
            unsigned int c2 = *i;
            rc += (((c1 - 'a') << 4) + (c2 - 'a'));
        }

        ++i;
    }
    return rc;
}

/**
 * Serialize test_result avoiding interfering with operator <<.
 */
void serialize(std::ostream &os, const tut::test_result &tr)
{
    os << escape(tr.group) << std::endl;
    os << tr.test << ' ';
    switch (tr.result)
    {
        case test_result::ok:
            os << 0;
            break;
        case test_result::fail:
            os << 1;
            break;
        case test_result::ex:
            os << 2;
            break;
        case test_result::warn:
            os << 3;
            break;
        case test_result::term:
            os << 4;
            break;
        case test_result::rethrown:
            os << 5;
            break;
        case test_result::ex_ctor:
            os << 6;
            break;
        case test_result::dummy:
            assert(!"Should never be called");
        default:
            throw std::logic_error("operator << : bad result_type");
    }
    os << ' ' << escape(tr.message) << std::endl;
}

/**
 * deserialization for test_result
 */
bool deserialize(std::istream &is, tut::test_result &tr)
{
    std::getline(is, tr.group);
    if (is.eof())
    {
        return false;
    }
    tr.group = unescape(tr.group);

    tr.test = -1;
    is >> tr.test;
    if (tr.test < 0)
    {
        throw std::logic_error("operator >> : bad test number");
    }

    int n = -1;
    is >> n;
    switch (n)
    {
        case 0:
            tr.result = test_result::ok;
            break;
        case 1:
            tr.result = test_result::fail;
            break;
        case 2:
            tr.result = test_result::ex;
            break;
        case 3:
            tr.result = test_result::warn;
            break;
        case 4:
            tr.result = test_result::term;
            break;
        case 5:
            tr.result = test_result::rethrown;
            break;
        case 6:
            tr.result = test_result::ex_ctor;
            break;
        default:
            throw std::logic_error("operator >> : bad result_type");
    }

    is.ignore(1);  // space
    std::getline(is, tr.message);
    tr.message = unescape(tr.message);
    if (!is.good())
    {
        throw std::logic_error("malformed test result");
    }
    return true;
}
};  // namespace util

/**
 * Restartable test runner wrapper.
 */
class restartable_wrapper
{
    test_runner &runner_;
    callbacks callbacks_;

    std::string dir_;
    std::string log_;  // log file: last test being executed
    std::string jrn_;  // journal file: results of all executed tests

  public:
    /**
     * Default constructor.
     * @param dir Directory where to search/put log and journal files
     */
    restartable_wrapper(const std::string &dir = ".")
        : runner_(runner.get()), dir_(dir)
    {
        // dozen: it works, but it would be better to use system path separator
        jrn_ = dir_ + '/' + "journal.tut";
        log_ = dir_ + '/' + "log.tut";
    }

    /**
     * Stores another group for getting by name.
     */
    void register_group(const std::string &name, group_base *gr)
    {
        runner_.register_group(name, gr);
    }

    /**
     * Stores callback object.
     */
    void set_callback(callback *cb)
    {
        callbacks_.clear();
        callbacks_.insert(cb);
    }

    void insert_callback(callback *cb)
    {
        callbacks_.insert(cb);
    }

    void erase_callback(callback *cb)
    {
        callbacks_.erase(cb);
    }

    void set_callbacks(const callbacks &cb)
    {
        callbacks_ = cb;
    }

    const callbacks &get_callbacks() const
    {
        return runner_.get_callbacks();
    }

    /**
     * Returns list of known test groups.
     */
    groupnames list_groups() const
    {
        return runner_.list_groups();
    }

    /**
     * Runs all tests in all groups.
     */
    void run_tests() const
    {
        // where last run was failed
        std::string fail_group;
        int fail_test;
        read_log_(fail_group, fail_test);
        bool fail_group_reached = (fail_group == "");

        // iterate over groups
        tut::groupnames gn = list_groups();
        tut::groupnames::const_iterator gni, gne;
        gni = gn.begin();
        gne = gn.end();
        while (gni != gne)
        {
            // skip all groups before one that failed
            if (!fail_group_reached)
            {
                if (*gni != fail_group)
                {
                    ++gni;
                    continue;
                }
                fail_group_reached = true;
            }

            // first or restarted run
            int test =
                (*gni == fail_group && fail_test >= 0) ? fail_test + 1 : 1;
            while (true)
            {
                // last executed test pos
                register_execution_(*gni, test);

                tut::test_result tr;
                if (!runner_.run_test(*gni, test, tr) ||
                    tr.result == test_result::dummy)
                {
                    break;
                }
                register_test_(tr);

                ++test;
            }

            ++gni;
        }

        // show final results to user
        invoke_callback_();

        // truncate files as mark of successful finish
        truncate_();
    }

  private:
    /**
     * Shows results from journal file.
     */
    void invoke_callback_() const
    {
        runner_.set_callbacks(callbacks_);
        runner_.cb_run_started_();

        std::string current_group;
        std::ifstream ijournal(jrn_.c_str());
        while (ijournal.good())
        {
            tut::test_result tr;
            if (!util::deserialize(ijournal, tr))
            {
                break;
            }
            runner_.cb_test_completed_(tr);
        }

        runner_.cb_run_completed_();
    }

    /**
     * Register test into journal.
     */
    void register_test_(const test_result &tr) const
    {
        std::ofstream ojournal(jrn_.c_str(), std::ios::app);
        util::serialize(ojournal, tr);
        ojournal << std::flush;
        if (!ojournal.good())
        {
            throw std::runtime_error("unable to register test result in file " +
                                     jrn_);
        }
    }

    /**
     * Mark the fact test going to be executed
     */
    void register_execution_(const std::string &grp, int test) const
    {
        // last executed test pos
        std::ofstream olog(log_.c_str());
        olog << util::escape(grp) << std::endl
             << test << std::endl
             << std::flush;
        if (!olog.good())
        {
            throw std::runtime_error("unable to register execution in file " +
                                     log_);
        }
    }

    /**
     * Truncate tests.
     */
    void truncate_() const
    {
        std::ofstream olog(log_.c_str());
        std::ofstream ojournal(jrn_.c_str());
    }

    /**
     * Read log file
     */
    void read_log_(std::string &fail_group, int &fail_test) const
    {
        // read failure point, if any
        std::ifstream ilog(log_.c_str());
        std::getline(ilog, fail_group);
        fail_group = util::unescape(fail_group);
        ilog >> fail_test;
        if (!ilog.good())
        {
            fail_group = "";
            fail_test = -1;
            truncate_();
        }
        else
        {
            // test was terminated...
            tut::test_result tr(fail_group, fail_test, "",
                                tut::test_result::term);
            register_test_(tr);
        }
    }
};

}  // namespace tut

#endif
