#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Combat_TacticalFormationManager.generated.h"

USTRUCT(BlueprintType)
struct FCombat_FormationPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FVector RelativePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString RoleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    int32 Priority;

    FCombat_FormationPosition()
    {
        RelativePosition = FVector::ZeroVector;
        RoleName = TEXT("Default");
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct FCombat_TacticalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    FString FormationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FCombat_FormationPosition> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ECombat_TacticalState RequiredState;

    FCombat_TacticalFormation()
    {
        FormationName = TEXT("Default");
        FormationRadius = 500.0f;
        RequiredState = ECombat_TacticalState::Patrol;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TacticalFormationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalFormationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    TArray<FCombat_TacticalFormation> AvailableFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    FCombat_TacticalFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    TArray<AActor*> FormationMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    AActor* FormationLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    FVector FormationCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    float FormationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation Management")
    bool bIsFormationActive;

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void InitializeFormation(const FString& FormationName, const TArray<AActor*>& Members, AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void ChangeFormation(const FString& NewFormationName);

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void AddFormationMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void RemoveFormationMember(AActor* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    FVector GetAssignedPosition(AActor* Member) const;

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    bool IsFormationIntact() const;

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void BreakFormation();

    UFUNCTION(BlueprintCallable, Category = "Formation Control")
    void ReformFormation();

private:
    void SetupDefaultFormations();
    void AssignPositionsToMembers();
    FCombat_TacticalFormation* FindFormationByName(const FString& FormationName);
    void ValidateFormationIntegrity();

    float LastUpdateTime;
    TMap<AActor*, FCombat_FormationPosition> MemberAssignments;
};