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
                sh 'make'
            }
        }
        stage('check')
        {
            steps
            {
                cppcheck("src")
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
