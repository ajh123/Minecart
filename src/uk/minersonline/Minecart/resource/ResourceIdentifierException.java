package uk.minersonline.Minecart.resource;

public class ResourceIdentifierException extends Exception {
	public enum Error {
		PATH_ERROR("Non [a-z0-9/._-] character in path of identifier"),
		NAMESPACE_ERROR("Non [a-z0-9._-] character in namespace of identifier");

		final String message;

		Error(String message) {
			this.message = message;
		}
	}

	public ResourceIdentifierException(Error error) {
		super(error.message);
	}
}
