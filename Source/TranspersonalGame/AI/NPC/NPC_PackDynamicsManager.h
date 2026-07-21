#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "../../SharedTypes.h"
#include "NPC_PackDynamicsManager.generated.h"

class ANPC_DinosaurAI;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TWeakObjectPtr<APawn> DinosaurPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    ENPC_PackRole PackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float DistanceFromLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float LoyaltyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsActive;

    FNPC_PackMember()
    {
        DinosaurPawn = nullptr;
        PackRole = ENPC_PackRole::Member;
        DistanceFromLeader = 0.0f;
        LoyaltyLevel = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PackFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    ENPC_PackFormationType FormationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    TArray<FVector> MemberPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float FormationSpacing;

    FNPC_PackFormation()
    {
        FormationType = ENPC_PackFormationType::Line;
        FormationRadius = 500.0f;
        FormationSpacing = 200.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PackDynamicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PackDynamicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void InitializePack(APawn* LeaderPawn, const TArray<APawn*>& MemberPawns);

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void AddPackMember(APawn* NewMember, ENPC_PackRole Role = ENPC_PackRole::Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void RemovePackMember(APawn* MemberToRemove);

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void ChangePackLeader(APawn* NewLeader);

    // Formation Management
    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void SetPackFormation(ENPC_PackFormationType NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    FVector GetMemberTargetPosition(APawn* Member) const;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void IssuePackCommand(ENPC_PackCommand Command, const FVector& TargetLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void UpdatePackCohesion(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    void HandleMemberThreat(APawn* ThreatenedMember, AActor* ThreatSource);

    // Pack State Queries
    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    bool IsPackLeader(APawn* Pawn) const;

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    bool IsPackMember(APawn* Pawn) const;

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    int32 GetPackSize() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    float GetPackCohesion() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    APawn* GetPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack Dynamics")
    TArray<APawn*> GetPackMembers() const;

protected:
    // Pack Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<APawn> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    FNPC_PackFormation CurrentFormation;

    // Pack Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float PackCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float PackSeparationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    float LeadershipTransferCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    bool bAutoFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings")
    bool bEnablePackHunting;

private:
    // Internal Methods
    void UpdatePackMemberPositions(float DeltaTime);
    void CheckPackCohesion();
    void HandlePackCommunication();
    FVector CalculateFormationPosition(int32 MemberIndex) const;
    void ValidatePackMembers();

    // Timers
    float LastCohesionCheck;
    float LastLeadershipChange;
    float PackUpdateInterval;
};