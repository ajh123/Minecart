package uk.minersonline.Minecart.resource;

public class ResourceLoadingException extends Exception {
	public ResourceLoadingException() {}

	public ResourceLoadingException(String message) {
		super(message);
	}

	public ResourceLoadingException(String message, Throwable cause) {
		super(message, cause);
	}
}
