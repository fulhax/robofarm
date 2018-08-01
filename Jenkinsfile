pipeline
{
    agent any
    stages
    {
        stage('git')
        {
            steps
            {
                sh 'git submodule update --init --recursive'
            }
        }
        stage('build')
        {
            steps
            {
                sh 'make release'
            }
        }
        stage('check')
        {
            steps
            {
                cppcheckcommands()
                gccwarnings()
            }
        }
    }
    post
    {
        always
        {
            deleteDir() /* clean up our workspace */
        }
        success
        {
            slackstatus("success")
        }
        failure
        {
            slackstatus("fail")
        }
        unstable
        {
            slackstatus("unstable")
        }

    }
}
