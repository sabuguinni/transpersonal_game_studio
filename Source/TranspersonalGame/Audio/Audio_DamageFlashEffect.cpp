#include "Audio_DamageFlashEffect.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"

AAudio_DamageFlashEffect::AAudio_DamageFlashEffect()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create flash mesh component (fullscreen quad)
    FlashMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashMeshComponent"));
    FlashMeshComponent->SetupAttachment(RootComponent);
    
    // Set up mesh for fullscreen overlay
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
    if (PlaneMesh.Succeeded())
    {
        FlashMeshComponent->SetStaticMesh(PlaneMesh.Object);
    }

    // Initialize flash state
    bIsFlashing = false;
    CurrentFlashTime = 0.0f;
    FlashDuration = 0.3f;
    MaxFlashOpacity = 0.6f;
    FadeSpeed = 3.0f;
    CurrentFlashColor = FLinearColor::Red;
    DynamicFlashMaterial = nullptr;

    // Set initial visibility
    FlashMeshComponent->SetVisibility(false);
    FlashMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAudio_DamageFlashEffect::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeFlashPresets();

    // Create dynamic material instance
    if (BaseDamageFlashMaterial)
    {
        DynamicFlashMaterial = UMaterialInstanceDynamic::Create(BaseDamageFlashMaterial, this);
        if (DynamicFlashMaterial)
        {
            FlashMeshComponent->SetMaterial(0, DynamicFlashMaterial);
        }
    }

    // Position the flash overlay in front of camera
    SetActorLocation(FVector(100.0f, 0.0f, 0.0f));
    SetActorScale3D(FVector(10.0f, 10.0f, 1.0f));
}

void AAudio_DamageFlashEffect::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsFlashing)
    {
        UpdateFlashEffect(DeltaTime);
    }
}

void AAudio_DamageFlashEffect::InitializeFlashPresets()
{
    // Physical damage - red flash
    FAudio_FlashSettings PhysicalFlash;
    PhysicalFlash.FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    PhysicalFlash.FlashDuration = 0.3f;
    PhysicalFlash.MaxOpacity = 0.5f;
    PhysicalFlash.FadeSpeed = 4.0f;
    DamageFlashPresets.Add(EAudio_DamageType::Physical, PhysicalFlash);

    // Fire damage - orange flash
    FAudio_FlashSettings FireFlash;
    FireFlash.FlashColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
    FireFlash.FlashDuration = 0.5f;
    FireFlash.MaxOpacity = 0.7f;
    FireFlash.FadeSpeed = 3.0f;
    DamageFlashPresets.Add(EAudio_DamageType::Fire, FireFlash);

    // Poison damage - green flash
    FAudio_FlashSettings PoisonFlash;
    PoisonFlash.FlashColor = FLinearColor(0.0f, 1.0f, 0.2f, 1.0f);
    PoisonFlash.FlashDuration = 0.4f;
    PoisonFlash.MaxOpacity = 0.4f;
    PoisonFlash.FadeSpeed = 2.5f;
    DamageFlashPresets.Add(EAudio_DamageType::Poison, PoisonFlash);

    // Bleeding damage - dark red flash
    FAudio_FlashSettings BleedingFlash;
    BleedingFlash.FlashColor = FLinearColor(0.8f, 0.0f, 0.0f, 1.0f);
    BleedingFlash.FlashDuration = 0.6f;
    BleedingFlash.MaxOpacity = 0.3f;
    BleedingFlash.FadeSpeed = 2.0f;
    DamageFlashPresets.Add(EAudio_DamageType::Bleeding, BleedingFlash);

    // Critical hit - bright white flash
    FAudio_FlashSettings CriticalFlash;
    CriticalFlash.FlashColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    CriticalFlash.FlashDuration = 0.2f;
    CriticalFlash.MaxOpacity = 0.8f;
    CriticalFlash.FadeSpeed = 6.0f;
    DamageFlashPresets.Add(EAudio_DamageType::Critical, CriticalFlash);
}

void AAudio_DamageFlashEffect::TriggerDamageFlash(EAudio_DamageType DamageType, float DamageAmount)
{
    if (!DamageFlashPresets.Contains(DamageType))
    {
        return;
    }

    FAudio_FlashSettings FlashSettings = DamageFlashPresets[DamageType];
    
    // Scale intensity based on damage amount
    float DamageIntensity = CalculateDamageIntensity(DamageAmount);
    
    CurrentFlashColor = FlashSettings.FlashColor;
    FlashDuration = FlashSettings.FlashDuration;
    MaxFlashOpacity = FlashSettings.MaxOpacity * DamageIntensity;
    FadeSpeed = FlashSettings.FadeSpeed;
    
    // Start the flash
    bIsFlashing = true;
    CurrentFlashTime = 0.0f;
    FlashMeshComponent->SetVisibility(true);
    
    SetFlashOpacity(MaxFlashOpacity);

    UE_LOG(LogTemp, Warning, TEXT("Damage flash triggered: Type=%d, Damage=%.1f, Intensity=%.2f"), 
           (int32)DamageType, DamageAmount, DamageIntensity);
}

void AAudio_DamageFlashEffect::TriggerCustomFlash(FLinearColor FlashColor, float Duration, float Opacity)
{
    CurrentFlashColor = FlashColor;
    FlashDuration = Duration;
    MaxFlashOpacity = FMath::Clamp(Opacity, 0.0f, 1.0f);
    FadeSpeed = 4.0f;
    
    bIsFlashing = true;
    CurrentFlashTime = 0.0f;
    FlashMeshComponent->SetVisibility(true);
    
    SetFlashOpacity(MaxFlashOpacity);
}

void AAudio_DamageFlashEffect::StopFlash()
{
    bIsFlashing = false;
    CurrentFlashTime = 0.0f;
    FlashMeshComponent->SetVisibility(false);
    SetFlashOpacity(0.0f);
}

void AAudio_DamageFlashEffect::UpdateFlashEffect(float DeltaTime)
{
    CurrentFlashTime += DeltaTime;
    
    if (CurrentFlashTime >= FlashDuration)
    {
        StopFlash();
        return;
    }

    // Calculate fade curve (quick flash, then fade out)
    float NormalizedTime = CurrentFlashTime / FlashDuration;
    float OpacityMultiplier;
    
    if (NormalizedTime < 0.1f)
    {
        // Quick flash to max opacity
        OpacityMultiplier = NormalizedTime / 0.1f;
    }
    else
    {
        // Fade out
        float FadeTime = (NormalizedTime - 0.1f) / 0.9f;
        OpacityMultiplier = 1.0f - (FadeTime * FadeSpeed * DeltaTime);
        OpacityMultiplier = FMath::Clamp(OpacityMultiplier, 0.0f, 1.0f);
    }
    
    float CurrentOpacity = MaxFlashOpacity * OpacityMultiplier;
    SetFlashOpacity(CurrentOpacity);
}

void AAudio_DamageFlashEffect::SetFlashOpacity(float Opacity)
{
    if (DynamicFlashMaterial)
    {
        FLinearColor FlashColorWithOpacity = CurrentFlashColor;
        FlashColorWithOpacity.A = Opacity;
        DynamicFlashMaterial->SetVectorParameterValue(TEXT("FlashColor"), FlashColorWithOpacity);
        DynamicFlashMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
    }
}

float AAudio_DamageFlashEffect::CalculateDamageIntensity(float DamageAmount)
{
    // Scale damage intensity from 0.3 to 1.0 based on damage amount
    float NormalizedDamage = FMath::Clamp(DamageAmount / 100.0f, 0.0f, 1.0f);
    return FMath::Lerp(0.3f, 1.0f, NormalizedDamage);
}