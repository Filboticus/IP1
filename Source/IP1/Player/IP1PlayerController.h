// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "IP1/GameModes/IP1TeamTypes.h"
#include "IP1PlayerController.generated.h"

class UInputMappingContext;

/**
 * 
 */
UCLASS()
class IP1_API AIP1PlayerController : public APlayerController
{
	GENERATED_BODY()

	AIP1PlayerController();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TeamSelectionWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> TeamSelectionWidget;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

public:
	// Called by the server to display team selection.
	UFUNCTION(Client, Reliable)
	void Client_ShowTeamSelection();
	
	UFUNCTION(Client, Reliable)
	void Client_TeamSelectionComplete();
	
	// Called by the UI when a player chooses a team.
	UFUNCTION(Server, Reliable)
	void Server_RequestTeamSelection(EIP1Team TeamID);
	
	UFUNCTION(BlueprintCallable, Category = "Team Selection")
	void SelectAllies();
	
	UFUNCTION(BlueprintCallable, Category = "Team Selection")
	void SelectAxis();
	
	// Combat role
	UFUNCTION(BlueprintCallable, Category = "Player|Role")
	void RequestRoleSelection(EIP1PlayerRole RequestedRole);

	UFUNCTION(Server, Reliable)
	void ServerRequestRoleSelection(EIP1PlayerRole RequestedRole);

	UFUNCTION(Client, Reliable)
	void ClientRoleSelectionRejected();
	

protected:

	virtual void BeginPlay() override;
	
	void ShowTeamSelection();
	
	void HideTeamSelection();


	
};
