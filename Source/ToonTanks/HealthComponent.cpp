// Copyright Denys Kryvytskyi. All Rights Reserved.

#include "HealthComponent.h"
#include "ToonTanksGameMode.h"

#include "Kismet/GameplayStatics.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamageTaken);

    ToonTanksGameMode = Cast<AToonTanksGameMode>(UGameplayStatics::GetGameMode(this));
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UHealthComponent::CalculateEffectiveDamage(float RawDamage, float ArmorRating) const
{
    if (RawDamage <= 0.0f) {
        return 0.0f;
    }

    const float ClampedArmor = FMath::Clamp(ArmorRating, 0.0f, 0.85f);
    return RawDamage * (1.0f - ClampedArmor);
}

void UHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f || CurrentHealth <= 0.0f) {
        return;
    }

    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
}

float UHealthComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void UHealthComponent::DamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f) {
        return;
    }

    const float EffectiveDamage = CalculateEffectiveDamage(Damage, Armor);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - EffectiveDamage);

    if (CurrentHealth <= 0.0f && ToonTanksGameMode) {
        ToonTanksGameMode->ActorDied(DamagedActor);
    }
}
