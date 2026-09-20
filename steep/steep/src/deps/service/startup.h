#pragma once

namespace cache {
	bool set_driver_load( std::uint32_t pid );
}

namespace service {
	class c_service {
	public:
		bool create( ) {
			{
				HANDLE existing = CreateFileW(
					L"\\\\.\\EBIoDispatch",
					GENERIC_READ | GENERIC_WRITE,
					0, nullptr, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL, nullptr );
				if ( existing && existing != INVALID_HANDLE_VALUE ) {
					CloseHandle( existing );
					logging::warn( oxorany( "driver already loaded, reusing existing instance" ) );
					m_reused_driver = true;
					return true;
				}
			}

			if ( !load_driver_privilege( true ) ) {
				logging::error( oxorany( "Could not enable driver loading privilege.\n" ) );
				return false;
			}

			utility::gen_rnd_str( m_driver_name );
			if ( !install_service( m_driver_name ) ) {
				load_driver_privilege( false );
				return false;
			}

			m_reused_driver = false;
			return true;
		}

		bool cleanup( ) {
			auto services_pid = utility::find_pid( oxorany( L"services.exe" ) );
			if ( !cache::set_driver_load( services_pid ) )
				return false;

			memset( driver_bytes, 0, sizeof( driver_bytes ) );
			return true;
		}

		void unload( ) {
			if ( !m_reused_driver )
				uninstall_services( );
			load_driver_privilege( false );
		}

	private:
		wchar_t m_driver_name[ 20 ]{ };
		bool    m_reused_driver{ false };
	};
}