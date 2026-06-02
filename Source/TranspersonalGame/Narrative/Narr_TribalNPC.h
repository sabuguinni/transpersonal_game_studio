#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Narr_NPCInteractionManager.h"
#include "Narr_TribalNPC.generated.h"

UENUM(BlueprintType)
enum class ENarr_TribalRole : uint8
{
    Elder       UMETA(DisplayName = "Tribal Elder"),
    Hunter      UMETA(DisplayName = "Tribal Hunter"),
    Scout       UMETA(DisplayName = "Tribal Scout"),
    Crafter     UMETA(DisplayName = "Tribal Crafter"),
    Gatherer    UMETA(DisplayName = "Tribal Gatherer")
};

USTRUCT(BlueprintType)
struct FNarr_TribalPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Friendliness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Caution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Knowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression;

    FNarr_TribalPersonality()
    {
        Friendliness = 0.7f;
        Caution = 0.6f;
        Knowledge = 0.5f;
        Aggression = 0.2f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ANarr_TribalNPC : public APawn
{
    GENERATED_BODY()

public:
    ANarr_TribalNPC();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NPCMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNarr_NPCInteractionManager* InteractionManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    ENarr_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FNarr_TribalPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    bool bIsAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    int32 TimesInteracted;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void InitializeAsRole(ENarr_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void SetupInteractions();

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    FString GetRoleBasedGreeting();

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void OnPlayerInteract(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void UpdateTrustLevel(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    bool CanProvideInformation();

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void SetAvailability(bool bAvailable);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};