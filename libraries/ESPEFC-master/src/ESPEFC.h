#ifndef ESPEFC_H
#define ESPEFC_H

class ESP8266WebServer;

class ESPEFC
{
  public:
    ESPEFC(bool serial_debug=false);

    void setup(ESP8266WebServer *server)
    {
      setup(server, NULL, NULL);
    }

    void setup(ESP8266WebServer *server, const char * path)
    {
      setup(server, path, NULL, NULL);
    }

    void setup(ESP8266WebServer *server, const char * username, const char * password)
    {
      setup(server, "/update", username, password);
    }

    void setup(ESP8266WebServer *server, const char * path, const char * username, const char * password);

    void updateCredentials(const char * username, const char * password)
    {
      _username = (char *)username;
      _password = (char *)password;
    }

  protected:
    void _setUpdaterError();

  private:
    bool _serial_output;
    ESP8266WebServer *_server;
    char * _username;
    char * _password;
    bool _authenticated;
    String _updaterError;
};


#endif
