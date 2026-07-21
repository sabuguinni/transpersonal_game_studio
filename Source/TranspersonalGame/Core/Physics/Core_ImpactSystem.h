#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Core_ImpactSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_ImpactType : uint8
{
    Light       UMETA(DisplayName = "Light Impact"),
    Medium      UMETA(DisplayName = "Medium Impact"),
    Heavy       UMETA(DisplayName = "Heavy Impact"),
    Explosive   UMETA(DisplayName = "Explosive Impact")
};

UENUM(BlueprintType)
enum class ECore_SurfaceType : uint8
{
    Default     UMETA(DisplayName = "Default"),
    Rock        UMETA(DisplayName = "Rock"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Wood        UMETA(DisplayName = "Wood"),
    Metal       UMETA(DisplayName = "Metal"),
    Water       UMETA(DisplayName = "Water"),
    Flesh       UMETA(DisplayName = "Flesh"),
    Bone        UMETA(DisplayName = "Bone")
};

USTRUCT(BlueprintType)
struct FCore_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    ECore_ImpactType ImpactType = ECore_ImpactType::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    ECore_SurfaceType SurfaceType = ECore_SurfaceType::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ImpactForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    AActor* ImpactCauser = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    UPrimitiveComponent* ImpactComponent = nullptr;

    FCore_ImpactData()
    {
        ImpactType = ECore_ImpactType::Medium;
        SurfaceType = ECore_SurfaceType::Default;
        ImpactForce = 1000.0f;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactCauser = nullptr;
        ImpactComponent = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FCore_ImpactEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* ParticleEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    USoundBase* SoundEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float SoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bCreateDecal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float DecalSize = 10.0f;

    FCore_ImpactEffect()
    {
        ParticleEffect = nullptr;
        SoundEffect = nullptr;
        EffectScale = 1.0f;
        SoundVolume = 1.0f;
        bCreateDecal = false;
        DecalSize = 10.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_ImpactSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_ImpactSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    TMap<ECore_SurfaceType, FCore_ImpactEffect> SurfaceEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    float MinImpactForce = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    float MaxImpactForce = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    bool bEnableImpactEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact System")
    float EffectCooldown = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Impact System")
    float LastImpactTime = 0.0f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ProcessImpact(const FCore_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void ProcessHitResult(const FHitResult& HitResult, float ImpactForce, AActor* Causer);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    ECore_SurfaceType GetSurfaceType(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    ECore_ImpactType GetImpactType(float Force);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void SpawnImpactEffects(const FCore_ImpactData& ImpactData);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void SetSurfaceEffect(ECore_SurfaceType SurfaceType, const FCore_ImpactEffect& Effect);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    FCore_ImpactEffect GetSurfaceEffect(ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    void EnableImpactEffects(bool bEnable) { bEnableImpactEffects = bEnable; }

    UFUNCTION(BlueprintCallable, Category = "Impact System")
    bool CanProcessImpact() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Impact System")
    void OnImpactProcessed(const FCore_ImpactData& ImpactData);

private:
    void InitializeDefaultEffects();
    void SpawnParticleEffect(const FCore_ImpactData& ImpactData, UParticleSystem* ParticleSystem, float Scale);
    void PlaySoundEffect(const FCore_ImpactData& ImpactData, USoundBase* Sound, float Volume);
    void CreateDecal(const FCore_ImpactData& ImpactData, float Size);
};