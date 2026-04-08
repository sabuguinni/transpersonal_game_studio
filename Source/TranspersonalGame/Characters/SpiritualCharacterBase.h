// SpiritualCharacterBase.h
// Base class for all spiritual/transcendental characters in the game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "SpiritualCharacterBase.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessLevel : uint8
{
    Unconscious     UMETA(DisplayName = "Unconscious"),
    Subconscious    UMETA(DisplayName = "Subconscious"),
    Conscious       UMETA(DisplayName = "Conscious"),
    Superconscious  UMETA(DisplayName = "Superconscious"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

UENUM(BlueprintType)
enum class EArchetypalRole : uint8
{
    Guide           UMETA(DisplayName = "Spiritual Guide"),
    Shadow          UMETA(DisplayName = "Shadow Aspect"),
    Seeker          UMETA(DisplayName = "Awakened Seeker"),
    Guardian        UMETA(DisplayName = "Sacred Guardian"),
    Trickster       UMETA(DisplayName = "Divine Trickster"),
    Healer          UMETA(DisplayName = "Soul Healer"),
    Teacher         UMETA(DisplayName = "Wisdom Teacher")
};

USTRUCT(BlueprintType)
struct FChakraState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    float EnergyLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    FLinearColor ChakraColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    float PulsationSpeed = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASpiritualCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    ASpiritualCharacterBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Character")
    EConsciousnessLevel ConsciousnessLevel = EConsciousnessLevel::Conscious;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Character")
    EArchetypalRole ArchetypalRole = EArchetypalRole::Guide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Character")
    float SpiritualEnergy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual Character")
    float TranscendenceLevel = 0.0f;

    // Aura System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aura")
    class UStaticMeshComponent* AuraMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    class UMaterialInterface* AuraMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    float AuraIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    float AuraRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aura")
    FLinearColor AuraColor = FLinearColor(0.8f, 0.9f, 1.0f, 0.3f);

    // Chakra System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakras")
    TArray<FChakraState> ChakraStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakras")
    bool bShowChakras = true;

    // Light Emanation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Light")
    class UPointLightComponent* SpiritualLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float LightIntensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    FLinearColor LightColor = FLinearColor(0.9f, 0.95f, 1.0f);

    // Particle Effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    class UParticleSystemComponent* EnergyParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* EnergyParticleSystem;

    // Animation and Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bCanLevitate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LevitationHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LevitationSpeed = 2.0f;

    // Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FString> WisdomQuotes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FString> GuidanceMessages;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void SetConsciousnessLevel(EConsciousnessLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void UpdateSpiritualEnergy(float DeltaEnergy);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void ActivateChakra(int32 ChakraIndex, bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void UpdateAura();

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void StartLevitation();

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void StopLevitation();

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    FString GetRandomWisdom();

    UFUNCTION(BlueprintCallable, Category = "Spiritual Character")
    void EmitSpiritualEnergy();

    UFUNCTION(BlueprintImplementableEvent, Category = "Spiritual Character")
    void OnConsciousnessLevelChanged();

    UFUNCTION(BlueprintImplementableEvent, Category = "Spiritual Character")
    void OnSpiritualEnergyChanged();

    UFUNCTION(BlueprintImplementableEvent, Category = "Spiritual Character")
    void OnTranscendenceAchieved();

private:
    void InitializeChakras();
    void UpdateChakraVisuals(float DeltaTime);
    void UpdateLevitation(float DeltaTime);
    
    float LevitationTimer = 0.0f;
    bool bIsLevitating = false;
    FVector OriginalLocation;
};