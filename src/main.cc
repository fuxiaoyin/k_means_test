#include "k_means.h"
#include "parameter.h"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        INFO("usage: k_means config_file!\n");
        return RET_ERROR;
    }
    Parameter param;
    param.load_conf(argv[1]);
    param.show();

    K_means k_means;
    k_means.init(param);
    k_means.do_cluster();
    k_means.write();

    return RET_OK;
}
